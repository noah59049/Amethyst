#include <iostream>
#include "ChessBoard.h"
#include "Flags.h"
#include "Bitmasks.h"
#include "../search/Zobrist.h"
#include "Logarithm.h"
#include "MagicBitboards.h"
#include "CheckBlock.h"
#include "../search/SEE.h"
#include "../Eval.h"

using namespace std;
using namespace bitmasks;

void ChessBoard::manuallyInitializeZobristCode() {
    // Step 0: Initizlize zobrist code to 0
    zobristCode = 0;
    // Step 1: Is it white to move
    if (isItWhiteToMove)
        zobristCode ^= zobrist::IS_IT_WHITE_TO_MOVE_CODE;
    // Step 2: Castling rights
    if (canWhiteCastleShort)
        zobristCode ^= zobrist::WHITE_CAN_CASTLE_SHORT_CODE;
    if (canBlackCastleShort)
        zobristCode ^= zobrist::BLACK_CAN_CASTLE_SHORT_CODE;
    if (canWhiteCastleLong)
        zobristCode ^= zobrist::WHITE_CAN_CASTLE_LONG_CODE;
    if (canBlackCastleLong)
        zobristCode ^= zobrist::BLACK_CAN_CASTLE_LONG_CODE;
    // Step 3: En passant rights
    zobristCode ^= zobrist::WHICH_PAWN_MOVED_TWO_SQUARES_CODES[whichPawnMovedTwoSquares];
    // Step 4: King positions
    zobristCode ^= zobrist::WHITE_KING_CODES[whiteKingPosition];
    zobristCode ^= zobrist::BLACK_KING_CODES[blackKingPosition];
    // Step 5: Piece positions
    for (int pieceType = QUEEN_CODE; pieceType <= PAWN_CODE; pieceType++) {
        bitboard_t whitePiecesRemaining = whitePieceTypes[pieceType];
        bitboard_t thisWhitePieceMask;
        bitboard_t thisWhitePieceSquare;
        while (whitePiecesRemaining != 0ULL) {
            thisWhitePieceMask = whitePiecesRemaining & -whitePiecesRemaining;
            whitePiecesRemaining -= thisWhitePieceMask;
            thisWhitePieceSquare = log2ll(thisWhitePieceMask);
            zobristCode ^= zobrist::WHITE_PIECE_TYPE_CODES[pieceType][thisWhitePieceSquare];
        }

        bitboard_t blackPiecesRemaining = blackPieceTypes[pieceType];
        bitboard_t thisBlackPieceMask;
        bitboard_t thisBlackPieceSquare;
        while (blackPiecesRemaining != 0ULL) {
            thisBlackPieceMask = blackPiecesRemaining & -blackPiecesRemaining;
            blackPiecesRemaining -= thisBlackPieceMask;
            thisBlackPieceSquare = log2ll(thisBlackPieceMask);
            zobristCode ^= zobrist::BLACK_PIECE_TYPE_CODES[pieceType][thisBlackPieceSquare];
        }
    } // end for loop over piece type
} // end manuallyInitializeZobristCode

ChessBoard::ChessBoard(const string& fenNotation) {
    int x = 0;
    int y = 7;

    int charPointer = 0;
    char currentChar;
    // Read the actual pieces

    whiteKingPosition = 64;
    blackKingPosition = 64;
    for (int pieceType = QUEEN_CODE; pieceType <= PAWN_CODE; pieceType++) {
        whitePieceTypes[pieceType] = 0;
        blackPieceTypes[pieceType] = 0;
    }
    do {
        currentChar = fenNotation[charPointer];
        charPointer++;

        switch (currentChar) {
            case 'K':
                whiteKingPosition = 8 * x + y;
                x++;
                break;
            case 'Q':
                whitePieceTypes[QUEEN_CODE] |= 1ULL << (8 * x + y);
                x++;
                break;
            case 'R':
                whitePieceTypes[ROOK_CODE] |= 1ULL << (8 * x + y);
                x++;
                break;
            case 'B':
                whitePieceTypes[BISHOP_CODE] |= 1ULL << (8 * x + y);
                x++;
                break;
            case 'N':
                whitePieceTypes[KNIGHT_CODE] |= 1ULL << (8 * x + y);
                x++;
                break;
            case 'P':
                whitePieceTypes[PAWN_CODE] |= 1ULL << (8 * x + y);
                x++;
                break;
            case 'k':
                blackKingPosition = 8 * x + y;
                x++;
                break;
            case 'q':
                blackPieceTypes[QUEEN_CODE] |= 1ULL << (8 * x + y);
                x++;
                break;
            case 'r':
                blackPieceTypes[ROOK_CODE] |= 1ULL << (8 * x + y);
                x++;
                break;
            case 'b':
                blackPieceTypes[BISHOP_CODE] |= 1ULL << (8 * x + y);
                x++;
                break;
            case 'n':
                blackPieceTypes[KNIGHT_CODE] |= 1ULL << (8 * x + y);
                x++;
                break;
            case 'p':
                blackPieceTypes[PAWN_CODE] |= 1ULL << (8 * x + y);
                x++;
                break;
            case '1':
                x += 1;
                break;
            case '2':
                x += 2;
                break;
            case '3':
                x += 3;
                break;
            case '4':
                x += 4;
                break;
            case '5':
                x += 5;
                break;
            case '6':
                x += 6;
                break;
            case '7':
                x += 7;
                break;
            case '8':
                x += 8;
                break;
            case '/':
                x = 0;
                y--;
                break;
            case ' ':
                break;
            default:
                cout << "Invalid FEN: Unexpected symbol in pieces: " << currentChar << endl;
                exit(1);
                break;
        }
    } while (currentChar != ' ');

    allWhitePieces = 1ULL << whiteKingPosition;
    allBlackPieces = 1ULL << blackKingPosition;
    for (int pieceType = QUEEN_CODE; pieceType <= PAWN_CODE; pieceType++) {
        allWhitePieces |= whitePieceTypes[pieceType];
        allBlackPieces |= blackPieceTypes[pieceType];
    }

    // now handle whose move it is
    currentChar = fenNotation[charPointer];
    charPointer++;

    switch (currentChar) {
        case 'w':
            isItWhiteToMove = true;
            break;
        case 'b':
            isItWhiteToMove = false;
            break;
        default:
            cout << "Invalid FEN: Unexpected character in whose turn it is" << endl;
            exit(1);
    }

    currentChar = fenNotation[charPointer];
    charPointer++;
    assert(currentChar == ' ');

    // handle who has castling rights where
    canWhiteCastleShort = false;
    canWhiteCastleLong = false;
    canBlackCastleShort = false;
    canBlackCastleLong = false;

    currentChar = fenNotation[charPointer];
    charPointer++;
    do {
        switch (currentChar) {
            case 'K':
                canWhiteCastleShort = true;
                break;
            case 'Q':
                canWhiteCastleLong = true;
                break;
            case 'k':
                canBlackCastleShort = true;
                break;
            case 'q':
                canBlackCastleLong = true;
                break;
            case '-':
                break;
            default:
                cout << "Invalid FEN: Unexpected character in castling rights: " << currentChar << endl;
                break;
        }
        currentChar = fenNotation[charPointer];
        charPointer++;
    } while (currentChar != ' ');

    currentChar = fenNotation[charPointer];
    charPointer++;

    whichPawnMovedTwoSquares = currentChar - 'a';
    if (whichPawnMovedTwoSquares > 7)
        whichPawnMovedTwoSquares = 255;

    drawByInsufficientMaterial = false;
    drawByStalemate = false;
    whiteWonByCheckmate = false;
    blackWonByCheckmate = false;

    // Check for draw by insufficient material
    updateDrawByInsufficientMaterial();
    manuallyInitializeZobristCode();
    updatePieceGivingCheck();
    updateMates();
}

move_t ChessBoard::getCaptureMove(const int startSquare, const int endSquare) const {
    if (isItWhiteToMove) {
        if (((blackPieceTypes[PAWN_CODE] >> endSquare) & 1ULL) == 1ULL)
            return (startSquare << 10) + (endSquare << 4) + CAPTURE_PAWN_FLAG;
        if (((blackPieceTypes[KNIGHT_CODE] >> endSquare) & 1ULL) == 1ULL)
            return (startSquare << 10) + (endSquare << 4) + CAPTURE_KNIGHT_FLAG;
        if (((blackPieceTypes[BISHOP_CODE] >> endSquare) & 1ULL) == 1ULL)
            return (startSquare << 10) + (endSquare << 4) + CAPTURE_BISHOP_FLAG;
        if (((blackPieceTypes[ROOK_CODE] >> endSquare) & 1ULL) == 1ULL)
            return (startSquare << 10) + (endSquare << 4) + CAPTURE_ROOK_FLAG;
        if (((blackPieceTypes[QUEEN_CODE] >> endSquare) & 1ULL) == 1ULL)
            return (startSquare << 10) + (endSquare << 4) + CAPTURE_QUEEN_FLAG;
    } else {
        if (((whitePieceTypes[PAWN_CODE] >> endSquare) & 1ULL) == 1ULL)
            return (startSquare << 10) + (endSquare << 4) + CAPTURE_PAWN_FLAG;
        if (((whitePieceTypes[KNIGHT_CODE] >> endSquare) & 1ULL) == 1ULL)
            return (startSquare << 10) + (endSquare << 4) + CAPTURE_KNIGHT_FLAG;
        if (((whitePieceTypes[BISHOP_CODE] >> endSquare) & 1ULL) == 1ULL)
            return (startSquare << 10) + (endSquare << 4) + CAPTURE_BISHOP_FLAG;
        if (((whitePieceTypes[ROOK_CODE] >> endSquare) & 1ULL) == 1ULL)
            return (startSquare << 10) + (endSquare << 4) + CAPTURE_ROOK_FLAG;
        if (((whitePieceTypes[QUEEN_CODE] >> endSquare) & 1ULL) == 1ULL)
            return (startSquare << 10) + (endSquare << 4) + CAPTURE_QUEEN_FLAG;
    }
    cout << toString() << endl << "is it white to move: " << isItWhiteToMove << endl;
    cout << "getCaptureMove isn't a move. Start square is " << startSquare << ". End square is " << endSquare << "." << endl;
    exit(1);
}

void ChessBoard::updateDrawByInsufficientMaterial() {
    // Scenario 1: K vs K, K + B vs K, or each side has bishops and they are all on the same color.
    bitboard_t allWhitePiecesExceptKings = allWhitePieces - (1ULL << whiteKingPosition);
    bitboard_t allBlackPiecesExceptKings = allBlackPieces - (1ULL << blackKingPosition);
    if (allWhitePiecesExceptKings == whitePieceTypes[BISHOP_CODE] and
        allBlackPiecesExceptKings == blackPieceTypes[BISHOP_CODE] and (
                ((allWhitePiecesExceptKings | allBlackPiecesExceptKings) & DARK_SQUARES) == DARK_SQUARES or
                ((allWhitePiecesExceptKings | allBlackPiecesExceptKings) & LIGHT_SQUARES) == LIGHT_SQUARES)
            )
        drawByInsufficientMaterial = true;

        // Scenario 2: K + N vs K
    else if (
            (whitePieceTypes[KNIGHT_CODE] == (-whitePieceTypes[KNIGHT_CODE] & whitePieceTypes[KNIGHT_CODE])) and
            (whitePieceTypes[KNIGHT_CODE] | (1ULL << whiteKingPosition)) == allWhitePieces and
            allBlackPieces == (1ULL << blackKingPosition))
        drawByInsufficientMaterial = true;

        // Scenario 3: K vs K + N
    else if ((1ULL << whiteKingPosition) == allWhitePieces and
             blackPieceTypes[KNIGHT_CODE] == (-blackPieceTypes[KNIGHT_CODE] & blackPieceTypes[KNIGHT_CODE]) and
             ((1ULL << blackKingPosition) | blackPieceTypes[KNIGHT_CODE]) == allBlackPieces)
        drawByInsufficientMaterial = true;
}


ChessBoard::ChessBoard() {
    // sets up the start position
    whiteKingPosition = 32;
    allWhitePieces = 0x0303030303030303ULL;

    blackKingPosition = whiteKingPosition + 7;
    allBlackPieces = 0xc0c0c0c0c0c0c0c0ULL;

    isItWhiteToMove = true;
    canWhiteCastleShort = true;
    canWhiteCastleLong = true;
    canBlackCastleShort = true;
    canBlackCastleLong = true;

    whichPawnMovedTwoSquares = 255;
    drawByInsufficientMaterial = false;
    drawByStalemate = false;
    whiteWonByCheckmate = false;
    blackWonByCheckmate = false;

    pieceGivingCheck = NOT_IN_CHECK_CODE;
    manuallyInitializeZobristCode();
}

//ChessBoard ChessBoard::boardFromFENNotation(const string& fenNotation) {
//    return ChessBoard(fenNotation);
//}

string ChessBoard::toString() const {
    string boardString;
    char piece;
    for (int rank = 7; rank >= 0; rank--) {
        string row = "+---+---+---+---+---+---+---+---+\n|";
        for (int file = 0; file < 8; file++) {
            int square = 8 * file + rank;
            bitboard_t squareMask = 1ULL << square;

            // Find what the piece is
            if (whiteKingPosition == square) {
                piece = 'K';
            } else if (blackKingPosition == square) {
                piece = 'k';
            } else if (squareMask & whitePieceTypes[QUEEN_CODE]) {
                piece = 'Q';
            } else if (squareMask & blackPieceTypes[QUEEN_CODE]) {
                piece = 'q';
            } else if (squareMask & whitePieceTypes[ROOK_CODE]) {
                piece = 'R';
            } else if (squareMask & blackPieceTypes[ROOK_CODE]) {
                piece = 'r';
            } else if (squareMask & whitePieceTypes[BISHOP_CODE]) {
                piece = 'B';
            } else if (squareMask & blackPieceTypes[BISHOP_CODE]) {
                piece = 'b';
            } else if (squareMask & whitePieceTypes[KNIGHT_CODE]) {
                piece = 'N';
            } else if (squareMask & blackPieceTypes[KNIGHT_CODE]) {
                piece = 'n';
            } else if (squareMask & whitePieceTypes[PAWN_CODE]) {
                piece = 'P';
            } else if (squareMask & blackPieceTypes[PAWN_CODE]) {
                piece = 'p';
            } else {
                piece = ' ';
            }
            // We are done finding the piece

            row += " " + string(1, piece) + " |";
        }
        row += " " + to_string(rank + 1) + "\n";
        boardString += row;
    }
    boardString += "+-a-+-b-+-c-+-d-+-e-+-f-+-g-+-h-+";
    return boardString;
}

uint8_t ChessBoard::calculatePieceGivingCheck() const {
    bitboard_t allPieces = allWhitePieces | allBlackPieces;

    uint8_t checker = NOT_IN_CHECK_CODE;

    if (isItWhiteToMove) {
        //bitboard_t blackPiecesGivingCheck;
        // Step 1: Pawns
        // Captures towards the a-file
        if ((((blackPieceTypes[PAWN_CODE] & NOT_A_FILE) >> (whiteKingPosition + 9)) & 1ULL) == 1ULL)
            checker = whiteKingPosition + 9;
            // Captures towards the h-file
        else if (((((blackPieceTypes[PAWN_CODE] & NOT_H_FILE) << 7) >> whiteKingPosition) & 1ULL) == 1ULL)
            checker = whiteKingPosition - 7;

        bitboard_t blackKnightsGivingCheck =
                getMagicKnightAttackedSquares(whiteKingPosition) & blackPieceTypes[KNIGHT_CODE];
        if (blackKnightsGivingCheck != 0ULL)
            checker = log2ll(blackKnightsGivingCheck);

        bitboard_t blackRooksGivingCheck = getMagicRookAttackedSquares(whiteKingPosition, allPieces) &
                                              (blackPieceTypes[ROOK_CODE] | blackPieceTypes[QUEEN_CODE]);
        if (blackRooksGivingCheck != 0ULL) {
            if (checker == NOT_IN_CHECK_CODE) {
                checker = log2ll(blackRooksGivingCheck);
                if (1ULL << checker != blackRooksGivingCheck)
                    return DOUBLE_CHECK_CODE;
            } else
                return DOUBLE_CHECK_CODE;
        }

        bitboard_t blackBishopsGivingCheck = getMagicBishopAttackedSquares(whiteKingPosition, allPieces) &
                                                (blackPieceTypes[BISHOP_CODE] | blackPieceTypes[QUEEN_CODE]);
        if (blackBishopsGivingCheck != 0ULL) {
            if (checker == NOT_IN_CHECK_CODE)
                return log2ll(blackBishopsGivingCheck);
            else
                return DOUBLE_CHECK_CODE;
        }

        return checker;
    } // end if it is white to move
    else {
        bitboard_t whitePiecesLeft;
        // Step 1: Pawns
        // Captures towards the a-file
        if (((((whitePieceTypes[PAWN_CODE] & NOT_A_FILE) >> 7) >> blackKingPosition) & 1ULL) == 1ULL)
            checker = blackKingPosition + 7;
            // Captures towards the h-file
        else if (((((whitePieceTypes[PAWN_CODE] & NOT_H_FILE) << 9) >> blackKingPosition) & 1ULL) == 1ULL)
            checker = blackKingPosition - 9;

        bitboard_t whiteKnightsGivingCheck =
                getMagicKnightAttackedSquares(blackKingPosition) & whitePieceTypes[KNIGHT_CODE];
        if (whiteKnightsGivingCheck != 0ULL)
            checker = log2ll(whiteKnightsGivingCheck);

        bitboard_t whiteRooksGivingCheck = getMagicRookAttackedSquares(blackKingPosition, allPieces) &
                                              (whitePieceTypes[ROOK_CODE] | whitePieceTypes[QUEEN_CODE]);
        if (whiteRooksGivingCheck != 0ULL) {
            if (checker == NOT_IN_CHECK_CODE) {
                checker = log2ll(whiteRooksGivingCheck);
                if (1ULL << checker != whiteRooksGivingCheck)
                    return DOUBLE_CHECK_CODE;
            } else
                return DOUBLE_CHECK_CODE;
        }

        bitboard_t whiteBishopsGivingCheck = getMagicBishopAttackedSquares(blackKingPosition, allPieces) &
                                                (whitePieceTypes[BISHOP_CODE] | whitePieceTypes[QUEEN_CODE]);
        if (whiteBishopsGivingCheck != 0ULL) {
            if (checker == NOT_IN_CHECK_CODE)
                return log2ll(whiteBishopsGivingCheck);
            else
                return DOUBLE_CHECK_CODE;
        }

        return checker;
    } // end if it is black to move
} // end calculatePieceGivingCheck

void ChessBoard::updatePieceGivingCheck() {
    pieceGivingCheck = calculatePieceGivingCheck();
}

bitboard_t ChessBoard::calculateWhiteAttackedSquares() const {
    // AKA: Squares where black can't move their king
    // This includes pinned pieces
    // This includes squares with the player's pieces
    // So if white has an unprotected knight, the square with the knight is not attacked because black can take that knight
    // However, if white has two knights protecting each other, they will both be included because black can't take either knight.
    bitboard_t attackedSquares = getMagicKingAttackedSquares(whiteKingPosition);
    bitboard_t meaningfulBlockers = (allWhitePieces | allBlackPieces) - (1ULL << blackKingPosition);

    bitboard_t whitePiecesRemaining;
    bitboard_t thisWhitePiece;

    for (int pieceType = QUEEN_CODE; pieceType <= KNIGHT_CODE; pieceType++) {
        whitePiecesRemaining = whitePieceTypes[pieceType];
        while (whitePiecesRemaining != 0ULL) {
            thisWhitePiece = whitePiecesRemaining & -whitePiecesRemaining;
            whitePiecesRemaining -= thisWhitePiece;
            attackedSquares |= getMagicWhiteAttackedSquares(pieceType, log2ll(thisWhitePiece),
                                                            meaningfulBlockers);
        }
    }

    // Step 5: Pawns
    attackedSquares |= (whitePieceTypes[PAWN_CODE] & NOT_A_FILE) >> 7;
    attackedSquares |= (whitePieceTypes[PAWN_CODE] & NOT_H_FILE) << 9;

    return attackedSquares;
}

bitboard_t ChessBoard::calculateBlackAttackedSquares() const {
    // AKA: Squares where white can't move their king
    // This includes pinned pieces
    // This includes squares with the player's pieces
    // So if white has an unprotected knight, the square with the knight is not attacked because black can take that knight
    // However, if white has two knights protecting each other, they will both be included because black can't take either knight.
    bitboard_t attackedSquares = getMagicKingAttackedSquares(blackKingPosition);
    bitboard_t meaningfulBlockers = (allWhitePieces | allBlackPieces) - (1ULL << whiteKingPosition);

    bitboard_t blackPiecesRemaining;
    bitboard_t thisBlackPiece;

    for (int pieceType = QUEEN_CODE; pieceType <= KNIGHT_CODE; pieceType++) {
        blackPiecesRemaining = blackPieceTypes[pieceType];
        while (blackPiecesRemaining != 0ULL) {
            thisBlackPiece = blackPiecesRemaining & -blackPiecesRemaining;
            blackPiecesRemaining -= thisBlackPiece;
            attackedSquares |= getMagicBlackAttackedSquares(pieceType, log2ll(thisBlackPiece),
                                                            meaningfulBlockers);
        }
    }

    // Step 5: Pawns
    attackedSquares |= (blackPieceTypes[PAWN_CODE] & NOT_A_FILE) >> 9;
    attackedSquares |= (blackPieceTypes[PAWN_CODE] & NOT_H_FILE) << 7;
    return attackedSquares;
}

void ChessBoard::makemove(move_t move) {
    if (whichPawnMovedTwoSquares < 8) {
        zobristCode ^= zobrist::WHICH_PAWN_MOVED_TWO_SQUARES_CODES[whichPawnMovedTwoSquares] ^
                       zobrist::WHICH_PAWN_MOVED_TWO_SQUARES_CODES[255];
        whichPawnMovedTwoSquares = 255;
    }
    move_t startSquare = move >> 10;
    move_t endSquare = (move >> 4) & 63;
    move_t flag = getFlag(move);
    bool isCapture = false;
    if (isItWhiteToMove) {
        if (((allBlackPieces >> endSquare) & 1ULL) == 1ULL) { // then this move is a capture
            isCapture = true;
            if (blackKingPosition == endSquare) {
                cout << toString() << endl << "Black king position equals end square; assertion failed" << endl;
                exit(1);
            }
            if (PROMOTE_TO_QUEEN_FLAG <= flag and flag <= PROMOTE_TO_KNIGHT_FLAG) {
                for (int capturedPieceType = QUEEN_CODE;
                     capturedPieceType <= KNIGHT_CODE; capturedPieceType++) {
                    if (((blackPieceTypes[capturedPieceType] >> endSquare) & 1ULL) ==
                        1ULL) { // this is the pieceType we are capturing
                        blackPieceTypes[capturedPieceType] -= 1ULL << endSquare;
                        allBlackPieces -= 1ULL << endSquare;
                        zobristCode ^= zobrist::BLACK_PIECE_TYPE_CODES[capturedPieceType][endSquare];
                        break;
                    }
                }
            } else {
                allBlackPieces -= 1ULL << endSquare;
                blackPieceTypes[flag - CAPTURE_QUEEN_FLAG + QUEEN_CODE] -= 1ULL << endSquare;
                zobristCode ^= zobrist::BLACK_PIECE_TYPE_CODES[flag - CAPTURE_QUEEN_FLAG +
                                                                       QUEEN_CODE][endSquare];
            }
        }

        if (move == WHITE_SHORT_CASTLE) {
            whiteKingPosition = 48;
            whitePieceTypes[ROOK_CODE] -= 72056494526300160; // add f1, get rid of h1
            allWhitePieces -= 71775023844556800; // add f1 and g1, get rid of e1 and h1
            canWhiteCastleShort = false;
            if (canWhiteCastleLong) {
                canWhiteCastleLong = false;
                zobristCode ^= zobrist::WHITE_CAN_CASTLE_LONG_CODE;
            }
            isItWhiteToMove = false;

            zobristCode ^= zobrist::WHITE_PIECE_TYPE_CODES[ROOK_CODE][40] ^
                           zobrist::WHITE_PIECE_TYPE_CODES[ROOK_CODE][56] ^
                           zobrist::WHITE_KING_CODES[32] ^ zobrist::WHITE_KING_CODES[48] ^
                           zobrist::WHITE_CAN_CASTLE_SHORT_CODE ^
                           zobrist::IS_IT_WHITE_TO_MOVE_CODE;

            updatePieceGivingCheck();
            updateMates();
            return;
            // We did this because when we're not castling, we update allWhitePieces at the very end, but if we were castling, we would update it in here.
            // So we don't want to update it twice.
        }
        else if (move == WHITE_LONG_CASTLE) {
            whiteKingPosition = 16;
            whitePieceTypes[ROOK_CODE] += 16777215; // add d1, get rid of a1
            allWhitePieces -= 4278124545; // add c1 and d1, get rid of e1 and a1
            if (canWhiteCastleShort) {
                canWhiteCastleShort = false;
                zobristCode ^= zobrist::WHITE_CAN_CASTLE_SHORT_CODE;
            }
            canWhiteCastleLong = false;
            isItWhiteToMove = false;

            zobristCode ^= zobrist::WHITE_PIECE_TYPE_CODES[ROOK_CODE][0] ^
                           zobrist::WHITE_PIECE_TYPE_CODES[ROOK_CODE][24] ^
                           zobrist::WHITE_KING_CODES[32] ^ zobrist::WHITE_KING_CODES[16] ^
                           zobrist::WHITE_CAN_CASTLE_LONG_CODE ^
                           zobrist::IS_IT_WHITE_TO_MOVE_CODE;

            updatePieceGivingCheck();
            updateMates();
            return;
            // We did this because when we're not castling, we update allWhitePieces at the very end, but if we were castling, we would update it in here.
            // So we don't want to update it twice.
        }
        else if (flag == EN_PASSANT_FLAG) {
            blackPieceTypes[PAWN_CODE] -= 1ULL << (endSquare - 1);
            allBlackPieces -= 1ULL << (endSquare - 1);
            zobristCode ^= zobrist::BLACK_PIECE_TYPE_CODES[PAWN_CODE][endSquare - 1];
            whitePieceTypes[PAWN_CODE] |= 1ULL << endSquare;
            whitePieceTypes[PAWN_CODE] -= 1ULL << startSquare;
            zobristCode ^= zobrist::WHITE_PIECE_TYPE_CODES[PAWN_CODE][startSquare] ^
                           zobrist::WHITE_PIECE_TYPE_CODES[PAWN_CODE][endSquare];
        }
        else if (PROMOTE_TO_QUEEN_FLAG <= flag and flag <= PROMOTE_TO_KNIGHT_FLAG) {
            whitePieceTypes[PAWN_CODE] -= 1ULL << startSquare;
            zobristCode ^= zobrist::WHITE_PIECE_TYPE_CODES[PAWN_CODE][startSquare];
            whitePieceTypes[QUEEN_CODE + flag - PROMOTE_TO_QUEEN_FLAG] |= 1ULL << endSquare;
            zobristCode ^= zobrist::WHITE_PIECE_TYPE_CODES[QUEEN_CODE + flag -
                                                                   PROMOTE_TO_QUEEN_FLAG][endSquare];
        }
        else {
            if (flag == PAWN_PUSH_TWO_SQUARES_FLAG) {
                whitePieceTypes[PAWN_CODE] += 6ULL << (startSquare & 56);
                zobristCode ^= zobrist::WHITE_PIECE_TYPE_CODES[PAWN_CODE][startSquare] ^
                               zobrist::WHITE_PIECE_TYPE_CODES[PAWN_CODE][endSquare];
                whichPawnMovedTwoSquares = startSquare >> 3;
                zobristCode ^= zobrist::WHICH_PAWN_MOVED_TWO_SQUARES_CODES[255] ^
                               zobrist::WHICH_PAWN_MOVED_TWO_SQUARES_CODES[whichPawnMovedTwoSquares];
            }
            else if (whiteKingPosition == startSquare) {
                whiteKingPosition = endSquare;
                zobristCode ^= zobrist::WHITE_KING_CODES[endSquare] ^
                               zobrist::WHITE_KING_CODES[startSquare];
                if (canWhiteCastleShort) {
                    canWhiteCastleShort = false;
                    zobristCode ^= zobrist::WHITE_CAN_CASTLE_SHORT_CODE;
                }
                if (canWhiteCastleLong) {
                    canWhiteCastleLong = false;
                    zobristCode ^= zobrist::WHITE_CAN_CASTLE_LONG_CODE;
                }
            }
            else if (((whitePieceTypes[QUEEN_CODE] >> startSquare) & 1ULL) == 1ULL) {
                whitePieceTypes[QUEEN_CODE] |= 1ULL << endSquare;
                whitePieceTypes[QUEEN_CODE] -= 1ULL << startSquare;
                zobristCode ^= zobrist::WHITE_PIECE_TYPE_CODES[QUEEN_CODE][startSquare] ^
                               zobrist::WHITE_PIECE_TYPE_CODES[QUEEN_CODE][endSquare];
            }
            else if (((whitePieceTypes[ROOK_CODE] >> startSquare) & 1ULL) == 1ULL) {
                whitePieceTypes[ROOK_CODE] |= 1ULL << endSquare;
                whitePieceTypes[ROOK_CODE] -= 1ULL << startSquare;
                zobristCode ^= zobrist::WHITE_PIECE_TYPE_CODES[ROOK_CODE][startSquare] ^
                               zobrist::WHITE_PIECE_TYPE_CODES[ROOK_CODE][endSquare];
            }
            else if (((whitePieceTypes[BISHOP_CODE] >> startSquare) & 1ULL) == 1ULL) {
                whitePieceTypes[BISHOP_CODE] |= 1ULL << endSquare;
                whitePieceTypes[BISHOP_CODE] -= 1ULL << startSquare;
                zobristCode ^= zobrist::WHITE_PIECE_TYPE_CODES[BISHOP_CODE][startSquare] ^
                               zobrist::WHITE_PIECE_TYPE_CODES[BISHOP_CODE][endSquare];
            }
            else if (((whitePieceTypes[KNIGHT_CODE] >> startSquare) & 1ULL) == 1ULL) {
                whitePieceTypes[KNIGHT_CODE] |= 1ULL << endSquare;
                whitePieceTypes[KNIGHT_CODE] -= 1ULL << startSquare;
                zobristCode ^= zobrist::WHITE_PIECE_TYPE_CODES[KNIGHT_CODE][startSquare] ^
                               zobrist::WHITE_PIECE_TYPE_CODES[KNIGHT_CODE][endSquare];
            }
            else if (((whitePieceTypes[PAWN_CODE] >> startSquare) & 1ULL) == 1ULL) {
                whitePieceTypes[PAWN_CODE] |= 1ULL << endSquare;
                whitePieceTypes[PAWN_CODE] -= 1ULL << startSquare;
                zobristCode ^= zobrist::WHITE_PIECE_TYPE_CODES[PAWN_CODE][startSquare] ^
                               zobrist::WHITE_PIECE_TYPE_CODES[PAWN_CODE][endSquare];
            }
            else {
                cout << toString() << endl <<"White to move. Attempted move " << move << endl;
                bool thereWasAPieceAtStartSquare = false;
                assert(thereWasAPieceAtStartSquare);
            }
        }
        allWhitePieces -= 1ULL << startSquare;
        allWhitePieces |= 1ULL << endSquare;
        isItWhiteToMove = false;
        zobristCode ^= zobrist::IS_IT_WHITE_TO_MOVE_CODE;
    } // end if it is white to move
    else {
        if (((allWhitePieces >> endSquare) & 1ULL) == 1ULL) { // then this move is a capture
            isCapture = true;
            if (whiteKingPosition == endSquare) {
                cout << toString() << endl << "White king position equals end square; assertion failed" << endl;
                exit(1);
            }
            if (PROMOTE_TO_QUEEN_FLAG <= flag and flag <= PROMOTE_TO_KNIGHT_FLAG) {
                for (int capturedPieceType = QUEEN_CODE;
                     capturedPieceType <= KNIGHT_CODE; capturedPieceType++) {
                    if (((whitePieceTypes[capturedPieceType] >> endSquare) & 1ULL) ==
                        1ULL) { // this is the pieceType we are capturing
                        whitePieceTypes[capturedPieceType] -= 1ULL << endSquare;
                        allWhitePieces -= 1ULL << endSquare;
                        zobristCode ^= zobrist::WHITE_PIECE_TYPE_CODES[capturedPieceType][endSquare];
                        break;
                    }
                }
            } else {
                allWhitePieces -= 1ULL << endSquare;
                whitePieceTypes[flag - CAPTURE_QUEEN_FLAG + QUEEN_CODE] -= 1ULL << endSquare;
                zobristCode ^= zobrist::WHITE_PIECE_TYPE_CODES[flag - CAPTURE_QUEEN_FLAG +
                                                                       QUEEN_CODE][endSquare];
            }
        }

        if (move == BLACK_SHORT_CASTLE) {
            blackKingPosition = 55;
            blackPieceTypes[ROOK_CODE] -= 9223231299366420480; // add f8, get rid of h8
            allBlackPieces -= 9187203052103270400; // add f8 and g8, get rid of e8 and h8
            canBlackCastleShort = false;
            if (canBlackCastleLong) {
                canBlackCastleLong = false;
                zobristCode ^= zobrist::BLACK_CAN_CASTLE_LONG_CODE;
            }
            isItWhiteToMove = true;

            zobristCode ^= zobrist::BLACK_PIECE_TYPE_CODES[ROOK_CODE][47] ^
                           zobrist::BLACK_PIECE_TYPE_CODES[ROOK_CODE][63] ^
                           zobrist::BLACK_KING_CODES[39] ^ zobrist::BLACK_KING_CODES[55] ^
                           zobrist::BLACK_CAN_CASTLE_SHORT_CODE ^
                           zobrist::IS_IT_WHITE_TO_MOVE_CODE;

            updatePieceGivingCheck();
            updateMates();
            return;
            // We did this because when we're not castling, we update allBlackPieces at the very end, but if we were castling, we would update it in here.
            // So we don't want to update it twice.
        } else if (move == BLACK_LONG_CASTLE) {
            blackKingPosition = 23;
            blackPieceTypes[ROOK_CODE] += 2147483520; // add d8, get rid of a8
            allBlackPieces -= 547599941760; // add c8 and d8, get rid of e8 and a8
            if (canBlackCastleShort) {
                canBlackCastleShort = false;
                zobristCode ^= zobrist::BLACK_CAN_CASTLE_SHORT_CODE;
            }
            canBlackCastleLong = false;
            isItWhiteToMove = true;

            zobristCode ^= zobrist::BLACK_PIECE_TYPE_CODES[ROOK_CODE][7] ^
                           zobrist::BLACK_PIECE_TYPE_CODES[ROOK_CODE][31] ^
                           zobrist::BLACK_KING_CODES[39] ^ zobrist::BLACK_KING_CODES[23] ^
                           zobrist::BLACK_CAN_CASTLE_LONG_CODE ^
                           zobrist::IS_IT_WHITE_TO_MOVE_CODE;

            updatePieceGivingCheck();
            updateMates();
            return;
            // We did this because when we're not castling, we update allBlackPieces at the very end, but if we were castling, we would update it in here.
            // So we don't want to update it twice.
        }
        else if (flag == EN_PASSANT_FLAG) {
            whitePieceTypes[PAWN_CODE] -= 2ULL << endSquare;
            allWhitePieces -= 2ULL << endSquare;
            zobristCode ^= zobrist::WHITE_PIECE_TYPE_CODES[PAWN_CODE][endSquare + 1];
            blackPieceTypes[PAWN_CODE] |= 1ULL << endSquare;
            blackPieceTypes[PAWN_CODE] -= 1ULL << startSquare;
            zobristCode ^= zobrist::BLACK_PIECE_TYPE_CODES[PAWN_CODE][endSquare] ^
                           zobrist::BLACK_PIECE_TYPE_CODES[PAWN_CODE][startSquare];
        }
        else if (PROMOTE_TO_QUEEN_FLAG <= flag and flag <= PROMOTE_TO_KNIGHT_FLAG) {
            blackPieceTypes[PAWN_CODE] -= 1ULL << startSquare;
            zobristCode ^= zobrist::BLACK_PIECE_TYPE_CODES[PAWN_CODE][startSquare];
            blackPieceTypes[QUEEN_CODE + flag - PROMOTE_TO_QUEEN_FLAG] |= 1ULL << endSquare;
            zobristCode ^= zobrist::BLACK_PIECE_TYPE_CODES[QUEEN_CODE + flag -
                                                                   PROMOTE_TO_QUEEN_FLAG][endSquare];
        }
        else {
            if (flag == PAWN_PUSH_TWO_SQUARES_FLAG) {
                blackPieceTypes[PAWN_CODE] -= 48ULL << (startSquare & 56);
                zobristCode ^= zobrist::BLACK_PIECE_TYPE_CODES[PAWN_CODE][endSquare] ^
                               zobrist::BLACK_PIECE_TYPE_CODES[PAWN_CODE][startSquare];
                whichPawnMovedTwoSquares = startSquare >> 3;
                zobristCode ^= zobrist::WHICH_PAWN_MOVED_TWO_SQUARES_CODES[255] ^
                               zobrist::WHICH_PAWN_MOVED_TWO_SQUARES_CODES[whichPawnMovedTwoSquares];
            }
            else if (blackKingPosition == startSquare) {
                blackKingPosition = endSquare;
                zobristCode ^= zobrist::BLACK_KING_CODES[endSquare] ^
                               zobrist::BLACK_KING_CODES[startSquare];
                if (canBlackCastleShort) {
                    canBlackCastleShort = false;
                    zobristCode ^= zobrist::BLACK_CAN_CASTLE_SHORT_CODE;
                }
                if (canBlackCastleLong) {
                    canBlackCastleLong = false;
                    zobristCode ^= zobrist::BLACK_CAN_CASTLE_LONG_CODE;
                }
            }
            else if (((blackPieceTypes[QUEEN_CODE] >> startSquare) & 1ULL) == 1ULL) {
                blackPieceTypes[QUEEN_CODE] |= 1ULL << endSquare;
                blackPieceTypes[QUEEN_CODE] -= 1ULL << startSquare;
                zobristCode ^= zobrist::BLACK_PIECE_TYPE_CODES[QUEEN_CODE][endSquare] ^
                               zobrist::BLACK_PIECE_TYPE_CODES[QUEEN_CODE][startSquare];
            }
            else if (((blackPieceTypes[ROOK_CODE] >> startSquare) & 1ULL) == 1ULL) {
                blackPieceTypes[ROOK_CODE] |= 1ULL << endSquare;
                blackPieceTypes[ROOK_CODE] -= 1ULL << startSquare;
                zobristCode ^= zobrist::BLACK_PIECE_TYPE_CODES[ROOK_CODE][endSquare] ^
                               zobrist::BLACK_PIECE_TYPE_CODES[ROOK_CODE][startSquare];
            }
            else if (((blackPieceTypes[BISHOP_CODE] >> startSquare) & 1ULL) == 1ULL) {
                blackPieceTypes[BISHOP_CODE] |= 1ULL << endSquare;
                blackPieceTypes[BISHOP_CODE] -= 1ULL << startSquare;
                zobristCode ^= zobrist::BLACK_PIECE_TYPE_CODES[BISHOP_CODE][endSquare] ^
                               zobrist::BLACK_PIECE_TYPE_CODES[BISHOP_CODE][startSquare];
            }
            else if (((blackPieceTypes[KNIGHT_CODE] >> startSquare) & 1ULL) == 1ULL) {
                blackPieceTypes[KNIGHT_CODE] |= 1ULL << endSquare;
                blackPieceTypes[KNIGHT_CODE] -= 1ULL << startSquare;
                zobristCode ^= zobrist::BLACK_PIECE_TYPE_CODES[KNIGHT_CODE][endSquare] ^
                               zobrist::BLACK_PIECE_TYPE_CODES[KNIGHT_CODE][startSquare];
            }
            else if (((blackPieceTypes[PAWN_CODE] >> startSquare) & 1ULL) == 1ULL) {
                blackPieceTypes[PAWN_CODE] |= 1ULL << endSquare;
                blackPieceTypes[PAWN_CODE] -= 1ULL << startSquare;
                zobristCode ^= zobrist::BLACK_PIECE_TYPE_CODES[PAWN_CODE][endSquare] ^
                               zobrist::BLACK_PIECE_TYPE_CODES[PAWN_CODE][startSquare];
            }
            else {
                cout << toString() << endl <<"Black to move. Attempted move " << move << endl;
                bool thereWasAPieceAtStartSquare = false;
                assert(thereWasAPieceAtStartSquare);
            }
        }
        allBlackPieces -= 1ULL << startSquare;
        allBlackPieces |= 1ULL << endSquare;
        isItWhiteToMove = true;
        zobristCode ^= zobrist::IS_IT_WHITE_TO_MOVE_CODE;
    } // end if it is black to move

    // Now take away castling rights if the rooks are taken or moved
    if ((startSquare == 0 or endSquare == 0) and canWhiteCastleLong) {
        canWhiteCastleLong = false;
        zobristCode ^= zobrist::WHITE_CAN_CASTLE_LONG_CODE;
    }
    if ((startSquare == 7 or endSquare == 7) and canBlackCastleLong) {
        canBlackCastleLong = false;
        zobristCode ^= zobrist::BLACK_CAN_CASTLE_LONG_CODE;
    }
    if ((startSquare == 56 or endSquare == 56) and canWhiteCastleShort) {
        canWhiteCastleShort = false;
        zobristCode ^= zobrist::WHITE_CAN_CASTLE_SHORT_CODE;
    }
    if ((startSquare == 63 or endSquare == 63) and canBlackCastleShort) {
        canBlackCastleShort = false;
        zobristCode ^= zobrist::BLACK_CAN_CASTLE_SHORT_CODE;
    }

    if (isCapture)
        updateDrawByInsufficientMaterial();
    updatePieceGivingCheck();
    updateMates();

} // end makemove method

void ChessBoard::addLegalKingMoves (vector<move_t>& legalMoves, bitboard_t kingLegalEndSquares) const { // Not castling
    const bitboard_t enemyPieces = isItWhiteToMove ? allBlackPieces : allWhitePieces;
    uint8_t myKingPosition = isItWhiteToMove ? whiteKingPosition : blackKingPosition;
    bitboard_t thisKingLegalEndSquareMask;
    int thisKingLegalEndSquare;
    while (kingLegalEndSquares != 0ULL) {
        thisKingLegalEndSquareMask = kingLegalEndSquares & -kingLegalEndSquares;
        kingLegalEndSquares -= thisKingLegalEndSquareMask;
        thisKingLegalEndSquare = log2ll(thisKingLegalEndSquareMask);
        if (thisKingLegalEndSquareMask & enemyPieces)
            legalMoves.push_back(getCaptureMove(myKingPosition,thisKingLegalEndSquare));
        else
            legalMoves.push_back(myKingPosition << 10 | thisKingLegalEndSquare << 4 | NORMAL_MOVE_FLAG);
    }
}

void ChessBoard::addCastling (vector<move_t>& legalMoves, const bitboard_t enemyAttackedSquares) const {
    bitboard_t allPieces = allBlackPieces | allWhitePieces;
    if (isItWhiteToMove) {
        if (canWhiteCastleShort and (enemyAttackedSquares & E1_F1_G1) == 0ULL and (allPieces & E1_THROUGH_H1) == E1_H1)
            legalMoves.push_back(WHITE_SHORT_CASTLE);
        if (canWhiteCastleLong and (enemyAttackedSquares & E1_D1_C1) == 0ULL and (allPieces & E1_THROUGH_A1) == E1_A1)
            legalMoves.push_back(WHITE_LONG_CASTLE);
    }
    else {
        if (canBlackCastleShort and (enemyAttackedSquares & E8_F8_G8) == 0ULL and (allPieces & E8_THROUGH_H8) == E8_H8)
            legalMoves.push_back(BLACK_SHORT_CASTLE);
        if (canBlackCastleLong and (enemyAttackedSquares & E8_D8_C8) == 0ULL and (allPieces & E8_THROUGH_A8) == E8_A8)
            legalMoves.push_back(BLACK_LONG_CASTLE);
    }
}

void ChessBoard::addEnPassant (vector<move_t>& legalMoves, const bitboard_t effectiveEnemyBishops, const bitboard_t effectiveEnemyRooks) const {
    const bitboard_t allPieces = allWhitePieces | allBlackPieces;
    if (pieceGivingCheck == NOT_IN_CHECK_CODE) {
        // Now we add en passant
        if (isItWhiteToMove) {
            if (whichPawnMovedTwoSquares < 8) { // REMINDER: The king is not in check
                int endSquare = 8 * whichPawnMovedTwoSquares + 5;
                // Consider left en passant
                if (whichPawnMovedTwoSquares < 7) { // The pawn that pushed two squares is not on the h-file, so we can en passant left
                    int startSquare = 8 * whichPawnMovedTwoSquares + 12;
                    bitboard_t allPiecesAfterCapture = allPieces + (1ULL << endSquare) - (1ULL << (8 * whichPawnMovedTwoSquares + 4)) - (1ULL << startSquare);
                    if (((whitePieceTypes[PAWN_CODE] >> startSquare) & 1ULL) == 1ULL and
                        (getMagicBishopAttackedSquares(whiteKingPosition,allPiecesAfterCapture) & effectiveEnemyBishops) == 0ULL and
                        (getMagicRookAttackedSquares(whiteKingPosition,allPiecesAfterCapture) & effectiveEnemyRooks) == 0ULL) {
                        legalMoves.push_back((startSquare << 10) + (endSquare << 4) + EN_PASSANT_FLAG);
                    } // end if this en passant doesn't reveal an attack on our king
                } // end if we can left en passant

                // Consider right en passant
                if (whichPawnMovedTwoSquares > 0) { // The pawn that pushed two squares is not on the a-file, so we can en passant right.
                    int startSquare = 8 * whichPawnMovedTwoSquares - 4;
                    bitboard_t allPiecesAfterCapture = allPieces + (1ULL << endSquare) - (1ULL << (8 * whichPawnMovedTwoSquares + 4)) - (1ULL << startSquare);
                    if (((whitePieceTypes[PAWN_CODE] >> startSquare) & 1ULL) == 1ULL and
                        (getMagicBishopAttackedSquares(whiteKingPosition,allPiecesAfterCapture) & effectiveEnemyBishops) == 0ULL and
                        (getMagicRookAttackedSquares(whiteKingPosition,allPiecesAfterCapture) & effectiveEnemyRooks) == 0ULL) {
                        legalMoves.push_back((startSquare << 10) + (endSquare << 4) + EN_PASSANT_FLAG);
                    } // end if this en passant doesn't reveal an attack on our king
                } // end if we can right en passant
            } // end if we can en passant
        } // end if it is white to move
        else {
            if (whichPawnMovedTwoSquares < 8) { // REMINDER: The king is not in check
                int endSquare = 8 * whichPawnMovedTwoSquares + 2;
                // Consider left en passant
                if (whichPawnMovedTwoSquares < 7) { // The pawn that pushed two squares is not on the h-file, so we can en passant left
                    int startSquare = 8 * whichPawnMovedTwoSquares + 11;
                    bitboard_t allPiecesAfterCapture = allPieces + (1ULL << endSquare) - (1ULL << (8 * whichPawnMovedTwoSquares + 3)) - (1ULL << startSquare);
                    if (((blackPieceTypes[PAWN_CODE] >> startSquare) & 1ULL) == 1ULL and
                        (getMagicBishopAttackedSquares(blackKingPosition,allPiecesAfterCapture) & effectiveEnemyBishops) == 0ULL and
                        (getMagicRookAttackedSquares(blackKingPosition,allPiecesAfterCapture) & effectiveEnemyRooks) == 0ULL) {
                        legalMoves.push_back((startSquare << 10) + (endSquare << 4) + EN_PASSANT_FLAG);
                    } // end if this en passant doesn't reveal an attack on our king
                } // end if we can left en passant

                // Consider right en passant
                if (whichPawnMovedTwoSquares > 0) { // The pawn that pushed two squares is not on the a-file, so we can en passant right.
                    int startSquare = 8 * whichPawnMovedTwoSquares - 5;
                    bitboard_t allPiecesAfterCapture = allPieces + (1ULL << endSquare) - (1ULL << (8 * whichPawnMovedTwoSquares + 3)) - (1ULL << startSquare);
                    if (((blackPieceTypes[PAWN_CODE] >> startSquare) & 1ULL) == 1ULL and
                        (getMagicBishopAttackedSquares(blackKingPosition,allPiecesAfterCapture) & effectiveEnemyBishops) == 0ULL and
                        (getMagicRookAttackedSquares(blackKingPosition,allPiecesAfterCapture) & effectiveEnemyRooks) == 0ULL) {
                        legalMoves.push_back((startSquare << 10) + (endSquare << 4) + EN_PASSANT_FLAG);
                    } // end if this en passant doesn't reveal an attack on our king
                } // end if we can right en passant
            } // end if we can en passant
        } // end if it is black to move
    } // end if we are not in check

    else { // we are in single check
        if (isItWhiteToMove) {
            if (whichPawnMovedTwoSquares < 8 and pieceGivingCheck == 8 * whichPawnMovedTwoSquares + 4) { // REMINDER: We are in check by a pawn that we can en passant capture. En passant is never legal after a discovered check.
                int endSquare = 8 * whichPawnMovedTwoSquares + 5;
                // Left en passant
                if (whichPawnMovedTwoSquares < 7) { // the pawn that pushed 2 squares is not the h-pawn, so we can en passant left.
                    int startSquare = 8 * whichPawnMovedTwoSquares + 12;
                    bitboard_t allPiecesAfterCapture = allPieces + (1ULL << endSquare) - (1ULL << (8 * whichPawnMovedTwoSquares + 4)) - (1ULL << startSquare);\
                if (((whitePieceTypes[PAWN_CODE] >> startSquare) & 1ULL) == 1ULL and (getMagicRookAttackedSquares(whiteKingPosition,allPiecesAfterCapture) & effectiveEnemyRooks) == 0ULL)
                        legalMoves.push_back((startSquare << 10) + (endSquare << 4) + EN_PASSANT_FLAG);
                } // end if we can en passant left

                // Consider right en passant
                if (whichPawnMovedTwoSquares > 0) { // The pawn that pushed two squares is not on the a-file, so we can en passant right.
                    int startSquare = 8 * whichPawnMovedTwoSquares - 4;
                    bitboard_t allPiecesAfterCapture = allPieces + (1ULL << endSquare) - (1ULL << (8 * whichPawnMovedTwoSquares + 4)) - (1ULL << startSquare);
                    if (((whitePieceTypes[PAWN_CODE] >> startSquare) & 1ULL) == 1ULL and (getMagicRookAttackedSquares(whiteKingPosition,allPiecesAfterCapture) & effectiveEnemyRooks) == 0ULL) {
                        legalMoves.push_back((startSquare << 10) + (endSquare << 4) + EN_PASSANT_FLAG);
                    } // end if this en passant doesn't reveal an attack on our king
                } // end if we can right en passant
            } // end if we can en passant at all
        } // end if it is white to move
        else { // it is black to move
            if (whichPawnMovedTwoSquares < 8 and pieceGivingCheck == 8 * whichPawnMovedTwoSquares + 3) { // REMINDER: We are in check by a pawn that we can en passant capture. En passant is never legal after a discovered check.
                int endSquare = 8 * whichPawnMovedTwoSquares + 2;
                // Left en passant
                if (whichPawnMovedTwoSquares < 7) { // the pawn that pushed 2 squares is not the h-pawn, so we can en passant left.
                    int startSquare = 8 * whichPawnMovedTwoSquares + 11;
                    bitboard_t allPiecesAfterCapture = allPieces + (1ULL << endSquare) - (1ULL << (8 * whichPawnMovedTwoSquares + 3)) - (1ULL << startSquare);\
                if (((blackPieceTypes[PAWN_CODE] >> startSquare) & 1ULL) == 1ULL and (getMagicRookAttackedSquares(blackKingPosition,allPiecesAfterCapture) & effectiveEnemyRooks) == 0ULL)
                        legalMoves.push_back((startSquare << 10) + (endSquare << 4) + EN_PASSANT_FLAG);
                } // end if we can en passant left

                // Consider right en passant
                if (whichPawnMovedTwoSquares > 0) { // The pawn that pushed two squares is not on the a-file, so we can en passant right.
                    int startSquare = 8 * whichPawnMovedTwoSquares - 5;
                    bitboard_t allPiecesAfterCapture = allPieces + (1ULL << endSquare) - (1ULL << (8 * whichPawnMovedTwoSquares + 3)) - (1ULL << startSquare);
                    if (((blackPieceTypes[PAWN_CODE] >> startSquare) & 1ULL) == 1ULL and (getMagicRookAttackedSquares(blackKingPosition,allPiecesAfterCapture) & effectiveEnemyRooks) == 0ULL) {
                        legalMoves.push_back((startSquare << 10) + (endSquare << 4) + EN_PASSANT_FLAG);
                    } // end if this en passant doesn't reveal an attack on our king
                } // end if we can right en passant
            } // end if we can en passant at all
        } // end if it is black to move
    } // end else (if we are in single check)
} // end addEnPassant method

void ChessBoard::getLegalMoves (vector<move_t>& legalMoves) const {
    const bitboard_t diagonalSquaresFromKing = isItWhiteToMove ? getEmptyBoardMagicBishopAttackedSquares(whiteKingPosition) : getEmptyBoardMagicBishopAttackedSquares(blackKingPosition);
    const bitboard_t orthogonalSquaresFromKing = isItWhiteToMove ? getEmptyBoardMagicRookAttackedSquares(whiteKingPosition) : getEmptyBoardMagicRookAttackedSquares(blackKingPosition);
    const bitboard_t effectiveEnemyBishops = isItWhiteToMove ? diagonalSquaresFromKing & (blackPieceTypes[BISHOP_CODE] | blackPieceTypes[QUEEN_CODE]) : diagonalSquaresFromKing & (whitePieceTypes[BISHOP_CODE] | whitePieceTypes[QUEEN_CODE]);
    const bitboard_t effectiveEnemyRooks = isItWhiteToMove ? orthogonalSquaresFromKing & (blackPieceTypes[ROOK_CODE] | blackPieceTypes[QUEEN_CODE]) : orthogonalSquaresFromKing & (whitePieceTypes[ROOK_CODE] | whitePieceTypes[QUEEN_CODE]);
    const bitboard_t enemyAttackedSquares = isItWhiteToMove ? calculateBlackAttackedSquares() : calculateWhiteAttackedSquares();
    const uint8_t myKingPosition = isItWhiteToMove ? whiteKingPosition : blackKingPosition;
    const bitboard_t myPieces = isItWhiteToMove ? allWhitePieces : allBlackPieces;
    const bitboard_t enemyPieces = isItWhiteToMove ? allBlackPieces : allWhitePieces;
    const bitboard_t allPieces = allWhitePieces | allBlackPieces;

    bitboard_t bishopPinnedPieces = 0ULL;
    bitboard_t rookPinnedPieces = 0ULL;

    bitboard_t thisPinningPieceMask, interposingSquares, interposingOccupiedSquares;
    int thisPinningPieceSquare;
    bitboard_t effectiveEnemyBishops1 = effectiveEnemyBishops, effectiveEnemyRooks1 = effectiveEnemyRooks;
    while (effectiveEnemyBishops1 != 0ULL) {
        thisPinningPieceMask = effectiveEnemyBishops1 & -effectiveEnemyBishops1;
        effectiveEnemyBishops1 -= thisPinningPieceMask;
        thisPinningPieceSquare = log2ll(thisPinningPieceMask);
        interposingSquares = lookupCheckResponses(myKingPosition,thisPinningPieceSquare) - (1ULL << thisPinningPieceSquare);
        interposingOccupiedSquares = interposingSquares & allPieces;
        if ((1ULL << log2ll(interposingOccupiedSquares)) == interposingOccupiedSquares)
            bishopPinnedPieces |= interposingSquares;
        // In other words, if there is only one piece between the Bishop and the King, then it's pinned
        // And we might have caught an opponent's piece that could give a discovered attack if it were their turn, but that won't affect anything.
    } // end while loop
    while (effectiveEnemyRooks1 != 0ULL) {
        thisPinningPieceMask = effectiveEnemyRooks1 & -effectiveEnemyRooks1;
        effectiveEnemyRooks1 -= thisPinningPieceMask;
        thisPinningPieceSquare = log2ll(thisPinningPieceMask);
        interposingSquares = lookupCheckResponses(myKingPosition,thisPinningPieceSquare) - (1ULL << thisPinningPieceSquare);
        interposingOccupiedSquares = interposingSquares & allPieces;
        if ((1ULL << log2ll(interposingOccupiedSquares)) == interposingOccupiedSquares)
            rookPinnedPieces |= interposingSquares;
        // In other words, if there is only one piece between the Rook and the King, then it's pinned
        // And we might have caught an opponent's piece that could give a discovered attack if it were their turn, but that won't affect anything.
    } // end while loop

    const bitboard_t kingLegalEndSquares = getMagicKingAttackedSquares(myKingPosition) & ~enemyAttackedSquares & ~myPieces;
    addLegalKingMoves(legalMoves,kingLegalEndSquares);
    if (pieceGivingCheck == DOUBLE_CHECK_CODE)
        return;

    bitboard_t legalCheckBlockSquares = ENTIRE_BOARD;
    addEnPassant(legalMoves,effectiveEnemyBishops,effectiveEnemyRooks);
    if (pieceGivingCheck == NOT_IN_CHECK_CODE)
        addCastling(legalMoves,enemyAttackedSquares);
    else
        legalCheckBlockSquares = lookupCheckResponses(myKingPosition,this->pieceGivingCheck);

    bitboard_t piecesRemaining;
    bitboard_t startSquareMask;
    int startSquare;
    bitboard_t legalEndSquares;
    bitboard_t endSquareMask;
    int endSquare;

    // Loop over all piece types, except pawns.
    for (int pieceType = KNIGHT_CODE; pieceType >= QUEEN_CODE; pieceType--) {
        piecesRemaining = isItWhiteToMove ? whitePieceTypes[pieceType] : blackPieceTypes[pieceType];
        while (piecesRemaining != 0ULL) {
            startSquareMask = piecesRemaining & -piecesRemaining;
            piecesRemaining -= startSquareMask;
            startSquare = log2ll(startSquareMask);
            legalEndSquares = getMagicWhiteAttackedSquares(pieceType, startSquare, allPieces) & legalCheckBlockSquares & ~myPieces;
            if (bishopPinnedPieces & startSquareMask)
                legalEndSquares &= diagonalSquaresFromKing & getEmptyBoardMagicBishopAttackedSquares(startSquare);
            else if (rookPinnedPieces & startSquareMask)
                legalEndSquares &= orthogonalSquaresFromKing & getEmptyBoardMagicRookAttackedSquares(startSquare);

            while (legalEndSquares != 0ULL) {
                endSquareMask = legalEndSquares & -legalEndSquares;
                legalEndSquares -= endSquareMask;
                endSquare = log2ll(endSquareMask);
                if (enemyPieces & endSquareMask)
                    legalMoves.push_back(getCaptureMove(startSquare, endSquare));
                else
                    legalMoves.push_back(startSquare << 10 | endSquare << 4 | NORMAL_MOVE_FLAG);
            } // end while legalEndSquares is not 0
        } // end while piecesRemaining is not 0
    } // end for loop over all piece types

    const bitboard_t myPawns = isItWhiteToMove ? whitePieceTypes[PAWN_CODE] : blackPieceTypes[PAWN_CODE];
    const int leftCaptureOffset = isItWhiteToMove ? 7 : 9;
    const int rightCaptureOffset = isItWhiteToMove ? 9 : 7; // but we are right shifting instead of left shifting
    const int pawnSinglePushOffset = isItWhiteToMove ? 1 : -1;
    const int pawnDoublePushOffset = isItWhiteToMove ? 2 : -2;

    // Pawns that can left capture
    piecesRemaining = myPawns & (enemyPieces & legalCheckBlockSquares) << leftCaptureOffset & ~rookPinnedPieces & (~bishopPinnedPieces | effectiveEnemyBishops << leftCaptureOffset);
    while (piecesRemaining != 0) {
        startSquareMask = piecesRemaining & -piecesRemaining;
        piecesRemaining -= startSquareMask;
        startSquare = log2ll(startSquareMask);
        endSquare = startSquare - leftCaptureOffset;
        if ((endSquare & 7) == 7 or (endSquare & 7) == 0) {
            legalMoves.push_back(startSquare << 10 | endSquare << 4 | PROMOTE_TO_QUEEN_FLAG);
            legalMoves.push_back(startSquare << 10 | endSquare << 4 | PROMOTE_TO_ROOK_FLAG);
            legalMoves.push_back(startSquare << 10 | endSquare << 4 | PROMOTE_TO_BISHOP_FLAG);
            legalMoves.push_back(startSquare << 10 | endSquare << 4 | PROMOTE_TO_KNIGHT_FLAG);
        }
        else {
            legalMoves.push_back(getCaptureMove(startSquare,endSquare));
        }
    } // end left captures

    // Pawns that can right capture
    piecesRemaining = myPawns & (enemyPieces & legalCheckBlockSquares) >> rightCaptureOffset & ~rookPinnedPieces & (~bishopPinnedPieces | effectiveEnemyBishops >> rightCaptureOffset);
    while (piecesRemaining != 0) {
        startSquareMask = piecesRemaining & -piecesRemaining;
        piecesRemaining -= startSquareMask;
        startSquare = log2ll(startSquareMask);
        endSquare = startSquare + rightCaptureOffset;
        if ((endSquare & 7) == 7 or (endSquare & 7) == 0) {
            legalMoves.push_back(startSquare << 10 | endSquare << 4 | PROMOTE_TO_QUEEN_FLAG);
            legalMoves.push_back(startSquare << 10 | endSquare << 4 | PROMOTE_TO_ROOK_FLAG);
            legalMoves.push_back(startSquare << 10 | endSquare << 4 | PROMOTE_TO_BISHOP_FLAG);
            legalMoves.push_back(startSquare << 10 | endSquare << 4 | PROMOTE_TO_KNIGHT_FLAG);
        }
        else {
            legalMoves.push_back(getCaptureMove(startSquare,endSquare));
        }
    } // end right captures

    // Pawn pushes one square
    piecesRemaining = isItWhiteToMove ?
                      myPawns & (~allPieces & legalCheckBlockSquares) >> 1 & ~bishopPinnedPieces & (~rookPinnedPieces | 255ULL << (myKingPosition & 56)) :
                      myPawns & (~allPieces & legalCheckBlockSquares) << 1 & ~bishopPinnedPieces & (~rookPinnedPieces | 255ULL << (myKingPosition & 56));

    while (piecesRemaining != 0ULL) {
        startSquareMask = piecesRemaining & -piecesRemaining;
        piecesRemaining -= startSquareMask;
        startSquare = log2ll(startSquareMask);
        endSquare = startSquare + pawnSinglePushOffset;
        if ((endSquare & 7) == 7 or (endSquare & 7) == 0) {
            legalMoves.push_back(startSquare << 10 | endSquare << 4 | PROMOTE_TO_QUEEN_FLAG);
            legalMoves.push_back(startSquare << 10 | endSquare << 4 | PROMOTE_TO_ROOK_FLAG);
            legalMoves.push_back(startSquare << 10 | endSquare << 4 | PROMOTE_TO_BISHOP_FLAG);
            legalMoves.push_back(startSquare << 10 | endSquare << 4 | PROMOTE_TO_KNIGHT_FLAG);
        }
        else {
            legalMoves.push_back(startSquare << 10 | endSquare << 4 | NORMAL_MOVE_FLAG);
        }
    } // end pawn pushes one square

    // Pawn pushes two squares
    piecesRemaining = isItWhiteToMove ?
                      myPawns & SECOND_RANK & ~allPieces >> 1 & ~allPieces >> 2 & legalCheckBlockSquares >> 2 & ~bishopPinnedPieces & (~rookPinnedPieces | 255ULL << (myKingPosition & 56)) :
                      myPawns &SEVENTH_RANK & ~allPieces << 1 & ~allPieces << 2 & legalCheckBlockSquares << 2 & ~bishopPinnedPieces & (~rookPinnedPieces | 255ULL << (myKingPosition & 56));

    while (piecesRemaining != 0ULL) {
        startSquareMask = piecesRemaining & -piecesRemaining;
        piecesRemaining -= startSquareMask;
        startSquare = log2ll(startSquareMask);
        endSquare = startSquare + pawnDoublePushOffset;
        legalMoves.push_back(startSquare << 10 | endSquare << 4 | PAWN_PUSH_TWO_SQUARES_FLAG);
    } // end pawn pushes two squares
} // end getLegalMoves method

bool ChessBoard::areThereLegalMoves () const {
    const bitboard_t diagonalSquaresFromKing = isItWhiteToMove ? getEmptyBoardMagicBishopAttackedSquares(whiteKingPosition) : getEmptyBoardMagicBishopAttackedSquares(blackKingPosition);
    const bitboard_t orthogonalSquaresFromKing = isItWhiteToMove ? getEmptyBoardMagicRookAttackedSquares(whiteKingPosition) : getEmptyBoardMagicRookAttackedSquares(blackKingPosition);
    const bitboard_t effectiveEnemyBishops = isItWhiteToMove ? diagonalSquaresFromKing & (blackPieceTypes[BISHOP_CODE] | blackPieceTypes[QUEEN_CODE]) : diagonalSquaresFromKing & (whitePieceTypes[BISHOP_CODE] | whitePieceTypes[QUEEN_CODE]);
    const bitboard_t effectiveEnemyRooks = isItWhiteToMove ? orthogonalSquaresFromKing & (blackPieceTypes[ROOK_CODE] | blackPieceTypes[QUEEN_CODE]) : orthogonalSquaresFromKing & (whitePieceTypes[ROOK_CODE] | whitePieceTypes[QUEEN_CODE]);
    const bitboard_t enemyAttackedSquares = isItWhiteToMove ? calculateBlackAttackedSquares() : calculateWhiteAttackedSquares();
    const uint8_t myKingPosition = isItWhiteToMove ? whiteKingPosition : blackKingPosition;
    const bitboard_t myPieces = isItWhiteToMove ? allWhitePieces : allBlackPieces;
    const bitboard_t enemyPieces = isItWhiteToMove ? allBlackPieces : allWhitePieces;
    const bitboard_t allPieces = allWhitePieces | allBlackPieces;

    bitboard_t bishopPinnedPieces = 0ULL;
    bitboard_t rookPinnedPieces = 0ULL;

    bitboard_t thisPinningPieceMask, interposingSquares, interposingOccupiedSquares;
    int thisPinningPieceSquare;
    bitboard_t effectiveEnemyBishops1 = effectiveEnemyBishops, effectiveEnemyRooks1 = effectiveEnemyRooks;
    while (effectiveEnemyBishops1 != 0ULL) {
        thisPinningPieceMask = effectiveEnemyBishops1 & -effectiveEnemyBishops1;
        effectiveEnemyBishops1 -= thisPinningPieceMask;
        thisPinningPieceSquare = log2ll(thisPinningPieceMask);
        interposingSquares = lookupCheckResponses(myKingPosition,thisPinningPieceSquare) - (1ULL << thisPinningPieceSquare);
        interposingOccupiedSquares = interposingSquares & allPieces;
        if ((1ULL << log2ll(interposingOccupiedSquares)) == interposingOccupiedSquares)
            bishopPinnedPieces |= interposingSquares;
        // In other words, if there is only one piece between the Bishop and the King, then it's pinned
        // And we might have caught an opponent's piece that could give a discovered attack if it were their turn, but that won't affect anything.
    } // end while loop
    while (effectiveEnemyRooks1 != 0ULL) {
        thisPinningPieceMask = effectiveEnemyRooks1 & -effectiveEnemyRooks1;
        effectiveEnemyRooks1 -= thisPinningPieceMask;
        thisPinningPieceSquare = log2ll(thisPinningPieceMask);
        interposingSquares = lookupCheckResponses(myKingPosition,thisPinningPieceSquare) - (1ULL << thisPinningPieceSquare);
        interposingOccupiedSquares = interposingSquares & allPieces;
        if ((1ULL << log2ll(interposingOccupiedSquares)) == interposingOccupiedSquares)
            rookPinnedPieces |= interposingSquares;
        // In other words, if there is only one piece between the Rook and the King, then it's pinned
        // And we might have caught an opponent's piece that could give a discovered attack if it were their turn, but that won't affect anything.
    } // end while loop

    // Does the king have legal moves
    if ((getMagicKingAttackedSquares(myKingPosition) & ~enemyAttackedSquares & ~myPieces) != 0)
        return true;

    if (pieceGivingCheck == DOUBLE_CHECK_CODE)
        return false;

    const bitboard_t legalCheckBlockSquares = pieceGivingCheck == NOT_IN_CHECK_CODE ? ENTIRE_BOARD : lookupCheckResponses(myKingPosition,this->pieceGivingCheck);

    bitboard_t piecesRemaining;
    bitboard_t startSquareMask;
    int startSquare;
    bitboard_t legalEndSquares;

    // Loop over all piece types, except pawns.
    for (int pieceType = KNIGHT_CODE; pieceType >= QUEEN_CODE; pieceType--) {
        piecesRemaining = isItWhiteToMove ? whitePieceTypes[pieceType] : blackPieceTypes[pieceType];
        while (piecesRemaining != 0ULL) {
            startSquareMask = piecesRemaining & -piecesRemaining;
            piecesRemaining -= startSquareMask;
            startSquare = log2ll(startSquareMask);
            legalEndSquares = getMagicWhiteAttackedSquares(pieceType, startSquare, allPieces) & legalCheckBlockSquares & ~myPieces;
            if (bishopPinnedPieces & startSquareMask)
                legalEndSquares &= diagonalSquaresFromKing & getEmptyBoardMagicBishopAttackedSquares(startSquare);
            else if (rookPinnedPieces & startSquareMask)
                legalEndSquares &= orthogonalSquaresFromKing & getEmptyBoardMagicRookAttackedSquares(startSquare);

            if (legalEndSquares != 0)
                return true;
        } // end while piecesRemaining is not 0
    } // end for loop over all piece types

    const bitboard_t myPawns = isItWhiteToMove ? whitePieceTypes[PAWN_CODE] : blackPieceTypes[PAWN_CODE];
    const int leftCaptureOffset = isItWhiteToMove ? 7 : 9;
    const int rightCaptureOffset = isItWhiteToMove ? 9 : 7; // but we are right shifting instead of left shifting
    const int pawnSinglePushOffset = isItWhiteToMove ? 1 : -1;
    const int pawnDoublePushOffset = isItWhiteToMove ? 2 : -2;

    if (isItWhiteToMove ?
        myPawns & (~allPieces & legalCheckBlockSquares) >> 1 & ~bishopPinnedPieces & (~rookPinnedPieces | 255ULL << (myKingPosition & 56)) :
        myPawns & (~allPieces & legalCheckBlockSquares) << 1 & ~bishopPinnedPieces & (~rookPinnedPieces | 255ULL << (myKingPosition & 56)))
        return true;
    if (isItWhiteToMove ?
        myPawns & SECOND_RANK & ~allPieces >> 1 & ~allPieces >> 2 & legalCheckBlockSquares >> 2 & ~bishopPinnedPieces & (~rookPinnedPieces | 255ULL << (myKingPosition & 56)) :
        myPawns &SEVENTH_RANK & ~allPieces << 1 & ~allPieces << 2 & legalCheckBlockSquares << 2 & ~bishopPinnedPieces & (~rookPinnedPieces | 255ULL << (myKingPosition & 56)))
        return true;

    if (myPawns & (enemyPieces & legalCheckBlockSquares) << leftCaptureOffset & ~rookPinnedPieces & (~bishopPinnedPieces | effectiveEnemyBishops << leftCaptureOffset))
        return true;
    if (myPawns & (enemyPieces & legalCheckBlockSquares) >> rightCaptureOffset & ~rookPinnedPieces & (~bishopPinnedPieces | effectiveEnemyBishops >> rightCaptureOffset))
        return true;

    vector<move_t> enPassantMoves;
    addEnPassant(enPassantMoves,effectiveEnemyBishops,effectiveEnemyRooks);
    return !enPassantMoves.empty();
}

void ChessBoard::updateMates() {
    if (!areThereLegalMoves()) {
        if (pieceGivingCheck == NOT_IN_CHECK_CODE)
            drawByStalemate = true;
        else {
            if (isItWhiteToMove)
                blackWonByCheckmate = true;
            else
                whiteWonByCheckmate = true;
        }
    }
}

void ChessBoard::getLegalCapturesOnly (std::vector<move_t>& captures) const {
    const bitboard_t diagonalSquaresFromKing = isItWhiteToMove ? getEmptyBoardMagicBishopAttackedSquares(whiteKingPosition) : getEmptyBoardMagicBishopAttackedSquares(blackKingPosition);
    const bitboard_t orthogonalSquaresFromKing = isItWhiteToMove ? getEmptyBoardMagicRookAttackedSquares(whiteKingPosition) : getEmptyBoardMagicRookAttackedSquares(blackKingPosition);
    const bitboard_t effectiveEnemyBishops = isItWhiteToMove ? diagonalSquaresFromKing & (blackPieceTypes[BISHOP_CODE] | blackPieceTypes[QUEEN_CODE]) : diagonalSquaresFromKing & (whitePieceTypes[BISHOP_CODE] | whitePieceTypes[QUEEN_CODE]);
    const bitboard_t effectiveEnemyRooks = isItWhiteToMove ? orthogonalSquaresFromKing & (blackPieceTypes[ROOK_CODE] | blackPieceTypes[QUEEN_CODE]) : orthogonalSquaresFromKing & (whitePieceTypes[ROOK_CODE] | whitePieceTypes[QUEEN_CODE]);
    const bitboard_t enemyAttackedSquares = isItWhiteToMove ? calculateBlackAttackedSquares() : calculateWhiteAttackedSquares();
    const uint8_t myKingPosition = isItWhiteToMove ? whiteKingPosition : blackKingPosition;
    const bitboard_t myPieces = isItWhiteToMove ? allWhitePieces : allBlackPieces;
    const bitboard_t enemyPieces = isItWhiteToMove ? allBlackPieces : allWhitePieces;
    const bitboard_t allPieces = allWhitePieces | allBlackPieces;

    bitboard_t bishopPinnedPieces = 0ULL;
    bitboard_t rookPinnedPieces = 0ULL;

    bitboard_t thisPinningPieceMask, interposingSquares, interposingOccupiedSquares;
    int thisPinningPieceSquare;
    bitboard_t effectiveEnemyBishops1 = effectiveEnemyBishops, effectiveEnemyRooks1 = effectiveEnemyRooks;
    while (effectiveEnemyBishops1 != 0ULL) {
        thisPinningPieceMask = effectiveEnemyBishops1 & -effectiveEnemyBishops1;
        effectiveEnemyBishops1 -= thisPinningPieceMask;
        thisPinningPieceSquare = log2ll(thisPinningPieceMask);
        interposingSquares = lookupCheckResponses(myKingPosition,thisPinningPieceSquare) - (1ULL << thisPinningPieceSquare);
        interposingOccupiedSquares = interposingSquares & allPieces;
        if ((1ULL << log2ll(interposingOccupiedSquares)) == interposingOccupiedSquares)
            bishopPinnedPieces |= interposingSquares;
        // In other words, if there is only one piece between the Bishop and the King, then it's pinned
        // And we might have caught an opponent's piece that could give a discovered attack if it were their turn, but that won't affect anything.
    } // end while loop
    while (effectiveEnemyRooks1 != 0ULL) {
        thisPinningPieceMask = effectiveEnemyRooks1 & -effectiveEnemyRooks1;
        effectiveEnemyRooks1 -= thisPinningPieceMask;
        thisPinningPieceSquare = log2ll(thisPinningPieceMask);
        interposingSquares = lookupCheckResponses(myKingPosition,thisPinningPieceSquare) - (1ULL << thisPinningPieceSquare);
        interposingOccupiedSquares = interposingSquares & allPieces;
        if ((1ULL << log2ll(interposingOccupiedSquares)) == interposingOccupiedSquares)
            rookPinnedPieces |= interposingSquares;
        // In other words, if there is only one piece between the Rook and the King, then it's pinned
        // And we might have caught an opponent's piece that could give a discovered attack if it were their turn, but that won't affect anything.
    } // end while loop

    const bitboard_t kingLegalEndSquares = getMagicKingAttackedSquares(myKingPosition) & ~enemyAttackedSquares & enemyPieces;
    addLegalKingMoves(captures,kingLegalEndSquares);
    addEnPassant(captures,effectiveEnemyBishops,effectiveEnemyRooks);
    if (pieceGivingCheck == DOUBLE_CHECK_CODE)
        return;

    bitboard_t piecesRemaining;
    bitboard_t startSquareMask;
    int startSquare;
    bitboard_t legalEndSquares;
    bitboard_t endSquareMask;
    int endSquare;

    const bitboard_t legalCheckBlockSquares = pieceGivingCheck == NOT_IN_CHECK_CODE ? ENTIRE_BOARD : lookupCheckResponses(myKingPosition,this->pieceGivingCheck);

    const bitboard_t myPawns = isItWhiteToMove ? whitePieceTypes[PAWN_CODE] : blackPieceTypes[PAWN_CODE];
    const int leftCaptureOffset = isItWhiteToMove ? 7 : 9;
    const int rightCaptureOffset = isItWhiteToMove ? 9 : 7; // but we are right shifting instead of left shifting

    // Pawns that can left capture
    piecesRemaining = myPawns & (enemyPieces & legalCheckBlockSquares) << leftCaptureOffset & ~rookPinnedPieces & (~bishopPinnedPieces | effectiveEnemyBishops << leftCaptureOffset);
    while (piecesRemaining != 0) {
        startSquareMask = piecesRemaining & -piecesRemaining;
        piecesRemaining -= startSquareMask;
        startSquare = log2ll(startSquareMask);
        endSquare = startSquare - leftCaptureOffset;
        if ((endSquare & 7) == 7 or (endSquare & 7) == 0) {
            captures.push_back(startSquare << 10 | endSquare << 4 | PROMOTE_TO_QUEEN_FLAG);
            captures.push_back(startSquare << 10 | endSquare << 4 | PROMOTE_TO_ROOK_FLAG);
            captures.push_back(startSquare << 10 | endSquare << 4 | PROMOTE_TO_BISHOP_FLAG);
            captures.push_back(startSquare << 10 | endSquare << 4 | PROMOTE_TO_KNIGHT_FLAG);
        }
        else {
            captures.push_back(getCaptureMove(startSquare,endSquare));
        }
    } // end left captures

    // Pawns that can right capture
    piecesRemaining = myPawns & (enemyPieces & legalCheckBlockSquares) >> rightCaptureOffset & ~rookPinnedPieces & (~bishopPinnedPieces | effectiveEnemyBishops >> rightCaptureOffset);
    while (piecesRemaining != 0) {
        startSquareMask = piecesRemaining & -piecesRemaining;
        piecesRemaining -= startSquareMask;
        startSquare = log2ll(startSquareMask);
        endSquare = startSquare + rightCaptureOffset;
        if ((endSquare & 7) == 7 or (endSquare & 7) == 0) {
            captures.push_back(startSquare << 10 | endSquare << 4 | PROMOTE_TO_QUEEN_FLAG);
            captures.push_back(startSquare << 10 | endSquare << 4 | PROMOTE_TO_ROOK_FLAG);
            captures.push_back(startSquare << 10 | endSquare << 4 | PROMOTE_TO_BISHOP_FLAG);
            captures.push_back(startSquare << 10 | endSquare << 4 | PROMOTE_TO_KNIGHT_FLAG);
        }
        else {
            captures.push_back(getCaptureMove(startSquare,endSquare));
        }
    } // end right captures

    // Loop over all piece types, except pawns.
    for (int pieceType = KNIGHT_CODE; pieceType >= QUEEN_CODE; pieceType--) {
        piecesRemaining = isItWhiteToMove ? whitePieceTypes[pieceType] : blackPieceTypes[pieceType];
        while (piecesRemaining != 0ULL) {
            startSquareMask = piecesRemaining & -piecesRemaining;
            piecesRemaining -= startSquareMask;
            startSquare = log2ll(startSquareMask);
            legalEndSquares = getMagicWhiteAttackedSquares(pieceType, startSquare, allPieces) & legalCheckBlockSquares & enemyPieces;
            if (bishopPinnedPieces & startSquareMask)
                legalEndSquares &= diagonalSquaresFromKing & getEmptyBoardMagicBishopAttackedSquares(startSquare);
            else if (rookPinnedPieces & startSquareMask)
                legalEndSquares &= orthogonalSquaresFromKing & getEmptyBoardMagicRookAttackedSquares(startSquare);

            while (legalEndSquares != 0ULL) {
                endSquareMask = legalEndSquares & -legalEndSquares;
                legalEndSquares -= endSquareMask;
                endSquare = log2ll(endSquareMask);
                captures.push_back(getCaptureMove(startSquare, endSquare));
            } // end while legalEndSquares is not 0
        } // end while piecesRemaining is not 0
    } // end for loop over all piece types
} // end getLegalCapturesOnly

int ChessBoard::getCaptureSEE (const int capturingPieceType, const move_t captureMove) const {
    int startSquare = captureMove >> 10;
    int endSquare = (captureMove >> 4) & 63;
    int pieceTypeCaptured = getFlag(captureMove) - CAPTURE_QUEEN_FLAG;

    int whiteSEEMask;
    if (((getMagicKingAttackedSquares(whiteKingPosition) >> endSquare) & 1ULL) == 1ULL)
        whiteSEEMask = see::ONLY_KING_ATTACKING;
    else
        whiteSEEMask = 0;
    int blackSEEMask;
    if (((getMagicKingAttackedSquares(blackKingPosition) >> endSquare) & 1ULL) == 1ULL)
        blackSEEMask = see::ONLY_KING_ATTACKING;
    else
        blackSEEMask = 0;

    bitboard_t squaresExceptStartSquare = ~(1ULL << startSquare);
    bitboard_t effectiveAllPieces = (allWhitePieces | allBlackPieces) & squaresExceptStartSquare;

    bitboard_t effectiveWhiteBishops = (whitePieceTypes[BISHOP_CODE] | whitePieceTypes[QUEEN_CODE]) & ~(1ULL << endSquare) & ~(1ULL << startSquare);
    bitboard_t effectiveWhiteRooks = (whitePieceTypes[ROOK_CODE] | whitePieceTypes[QUEEN_CODE]) & ~(1ULL << endSquare) & ~(1ULL << startSquare);
    bitboard_t effectiveBlackBishops = (blackPieceTypes[BISHOP_CODE] | blackPieceTypes[QUEEN_CODE]) & ~(1ULL << endSquare) & ~(1ULL << startSquare);
    bitboard_t effectiveBlackRooks = (blackPieceTypes[ROOK_CODE] | blackPieceTypes[QUEEN_CODE]) & ~(1ULL << endSquare) & ~(1ULL << startSquare);

    bitboard_t whiteBlockers;
    bitboard_t whiteAttackers;
    bitboard_t blackBlockers;
    bitboard_t blackAttackers;
    bitboard_t thisPieceMask;
    for (int pieceType = QUEEN_CODE; pieceType <= PAWN_CODE; pieceType++) {
        // white
        whiteBlockers = effectiveAllPieces & ~whitePieceTypes[pieceType];
        // We don't count other pieces making a battery with it as blocking
        // We do count queens in front of bishops or rooks as blocking
        if (pieceType == QUEEN_CODE) {
            whiteBlockers &= ~(getMagicRookAttackedSquares(endSquare, 0) & whitePieceTypes[ROOK_CODE]);
            whiteBlockers &= ~(getMagicBishopAttackedSquares(endSquare, 0) & whitePieceTypes[BISHOP_CODE]);
        }
        else if (pieceType == ROOK_CODE or pieceType == BISHOP_CODE) {
            whiteBlockers &= ~whitePieceTypes[QUEEN_CODE];
        }
        whiteAttackers = whitePieceTypes[pieceType] & getMagicBlackAttackedSquares(pieceType,endSquare,whiteBlockers) & squaresExceptStartSquare;
        while (whiteAttackers != 0ULL) {
            thisPieceMask = whiteAttackers & -whiteAttackers;
            whiteAttackers -= thisPieceMask;
            // Now we put this piece into the SEE mask as long as it is not pinned.
            if ((getMagicRookAttackedSquares(whiteKingPosition,effectiveAllPieces - thisPieceMask) & effectiveBlackRooks) == 0ULL and (getMagicBishopAttackedSquares(whiteKingPosition,effectiveAllPieces - thisPieceMask) & effectiveBlackBishops) == 0ULL)
                whiteSEEMask += 1 << ((4 - (pieceType - QUEEN_CODE)) * 4);
            else
                blackSEEMask &= ~see::ONLY_KING_ATTACKING;
        }

        // black
        blackBlockers = effectiveAllPieces & ~blackPieceTypes[pieceType];
        // We don't count other pieces making a battery with it as blocking
        // We do count queens in front of bishops or rooks as blocking
        if (pieceType == QUEEN_CODE) {
            blackBlockers &= ~(getMagicRookAttackedSquares(endSquare, 0) & blackPieceTypes[ROOK_CODE]);
            blackBlockers &= ~(getMagicBishopAttackedSquares(endSquare, 0) & blackPieceTypes[BISHOP_CODE]);
        }
        else if (pieceType == ROOK_CODE or pieceType == BISHOP_CODE) {
            blackBlockers &= ~blackPieceTypes[QUEEN_CODE];
        }
        blackAttackers = blackPieceTypes[pieceType] & getMagicWhiteAttackedSquares(pieceType,endSquare,blackBlockers) & squaresExceptStartSquare;
        while (blackAttackers != 0ULL) {
            thisPieceMask = blackAttackers & -blackAttackers;
            blackAttackers -= thisPieceMask;
            // Now we put this piece into the SEE mask as long as it is not pinned.
            if ((getMagicRookAttackedSquares(blackKingPosition,effectiveAllPieces - thisPieceMask) & effectiveWhiteRooks) == 0ULL and (getMagicBishopAttackedSquares(blackKingPosition,effectiveAllPieces - thisPieceMask) & effectiveWhiteBishops) == 0ULL)
                blackSEEMask += 1 << ((4 - (pieceType - QUEEN_CODE)) * 4);
            else
                whiteSEEMask &= ~see::ONLY_KING_ATTACKING; // Effectively, white can't take with the king because a pinned piece is attacking.
        }
    } // end for loop over piece types
    if (isItWhiteToMove)
        return see::TEXTBOOK_PIECE_VALUES[pieceTypeCaptured] - see::getSEE(capturingPieceType, blackSEEMask, whiteSEEMask);
    else
        return see::TEXTBOOK_PIECE_VALUES[pieceTypeCaptured] - see::getSEE(capturingPieceType, whiteSEEMask, blackSEEMask);
} // end getCaptureSEE

int ChessBoard::getQuietSEE (const int movePieceType, const move_t quietMove) const {
    int startSquare = quietMove >> 10;
    int endSquare = (quietMove >> 4) & 63;

    int whiteSEEMask;
    if (((getMagicKingAttackedSquares(whiteKingPosition) >> endSquare) & 1ULL) == 1ULL)
        whiteSEEMask = see::ONLY_KING_ATTACKING;
    else
        whiteSEEMask = 0;
    int blackSEEMask;
    if (((getMagicKingAttackedSquares(blackKingPosition) >> endSquare) & 1ULL) == 1ULL)
        blackSEEMask = see::ONLY_KING_ATTACKING;
    else
        blackSEEMask = 0;

    bitboard_t squaresExceptStartSquare = ~(1ULL << startSquare);
    bitboard_t effectiveAllPieces = (allWhitePieces | allBlackPieces) & squaresExceptStartSquare;

    bitboard_t effectiveWhiteBishops = (whitePieceTypes[BISHOP_CODE] | whitePieceTypes[QUEEN_CODE]) & ~(1ULL << endSquare) & ~(1ULL << startSquare);
    bitboard_t effectiveWhiteRooks = (whitePieceTypes[ROOK_CODE] | whitePieceTypes[QUEEN_CODE]) & ~(1ULL << endSquare) & ~(1ULL << startSquare);
    bitboard_t effectiveBlackBishops = (blackPieceTypes[BISHOP_CODE] | blackPieceTypes[QUEEN_CODE]) & ~(1ULL << endSquare) & ~(1ULL << startSquare);
    bitboard_t effectiveBlackRooks = (blackPieceTypes[ROOK_CODE] | blackPieceTypes[QUEEN_CODE]) & ~(1ULL << endSquare) & ~(1ULL << startSquare);

    bitboard_t whiteBlockers;
    bitboard_t whiteAttackers;
    bitboard_t blackBlockers;
    bitboard_t blackAttackers;
    bitboard_t thisPieceMask;
    for (int pieceType = QUEEN_CODE; pieceType <= PAWN_CODE; pieceType++) {
        // white
        whiteBlockers = effectiveAllPieces & ~whitePieceTypes[pieceType];
        // We don't count other pieces making a battery with it as blocking
        if (pieceType == QUEEN_CODE) {
            whiteBlockers &= ~(getMagicRookAttackedSquares(endSquare, 0) & whitePieceTypes[ROOK_CODE]);
            whiteBlockers &= ~(getMagicBishopAttackedSquares(endSquare, 0) & whitePieceTypes[BISHOP_CODE]);
        }
        else if (pieceType == ROOK_CODE or pieceType == BISHOP_CODE) {
            whiteBlockers &= ~whitePieceTypes[QUEEN_CODE];
        }
        whiteAttackers = whitePieceTypes[pieceType] & getMagicBlackAttackedSquares(pieceType,endSquare,whiteBlockers) & squaresExceptStartSquare;
        while (whiteAttackers != 0ULL) {
            thisPieceMask = whiteAttackers & -whiteAttackers;
            whiteAttackers -= thisPieceMask;
            // Now we put this piece into the SEE mask as long as it is not pinned.
            if ((getMagicRookAttackedSquares(whiteKingPosition,effectiveAllPieces - thisPieceMask) & effectiveBlackRooks) == 0ULL and (getMagicBishopAttackedSquares(whiteKingPosition,effectiveAllPieces - thisPieceMask) & effectiveBlackBishops) == 0ULL)
                whiteSEEMask += 1 << ((4 - (pieceType - QUEEN_CODE)) * 4);
            else
                blackSEEMask &= ~see::ONLY_KING_ATTACKING;
        }

        // black
        blackBlockers = effectiveAllPieces & ~blackPieceTypes[pieceType];
        // We don't count other pieces making a battery with it as blocking
        if (pieceType == QUEEN_CODE) {
            blackBlockers &= ~(getMagicRookAttackedSquares(endSquare, 0) & blackPieceTypes[ROOK_CODE]);
            blackBlockers &= ~(getMagicBishopAttackedSquares(endSquare, 0) & blackPieceTypes[BISHOP_CODE]);
        }
        else if (pieceType == ROOK_CODE or pieceType == BISHOP_CODE) {
            blackBlockers &= ~blackPieceTypes[QUEEN_CODE];
        }
        blackAttackers = blackPieceTypes[pieceType] & getMagicWhiteAttackedSquares(pieceType,endSquare,blackBlockers) & squaresExceptStartSquare;
        while (blackAttackers != 0ULL) {
            thisPieceMask = blackAttackers & -blackAttackers;
            blackAttackers -= thisPieceMask;
            // Now we put this piece into the SEE mask as long as it is not pinned.
            if ((getMagicRookAttackedSquares(blackKingPosition,effectiveAllPieces - thisPieceMask) & effectiveWhiteRooks) == 0ULL and (getMagicBishopAttackedSquares(blackKingPosition,effectiveAllPieces - thisPieceMask) & effectiveWhiteBishops) == 0ULL)
                blackSEEMask += 1 << ((4 - (pieceType - QUEEN_CODE)) * 4);
            else
                whiteSEEMask &= ~see::ONLY_KING_ATTACKING; // Effectively, white can't take with the king because a pinned piece is attacking.
        }
    } // end for loop over piece types
    if (isItWhiteToMove)
        return -see::getSEE(movePieceType, blackSEEMask, whiteSEEMask);
    else
        return -see::getSEE(movePieceType, whiteSEEMask, blackSEEMask);
}

int ChessBoard::getQuietSEE (const move_t move) const {
    const int endSquare = move >> 4 & 63;
    const bitboard_t endSquareMask = 1ULL << endSquare;
    int movingPieceType;
    if (isItWhiteToMove) {
        if (whiteKingPosition == endSquare)
            return 0;
        else if (whitePieceTypes[QUEEN_CODE] & endSquare)
            movingPieceType = QUEEN_CODE;
        else if (whitePieceTypes[ROOK_CODE] & endSquare)
            movingPieceType = ROOK_CODE;
        else if (whitePieceTypes[BISHOP_CODE] & endSquare)
            movingPieceType = BISHOP_CODE;
        else if (whitePieceTypes[KNIGHT_CODE] & endSquare)
            movingPieceType = KNIGHT_CODE;
        else if (whitePieceTypes[PAWN_CODE] & endSquare)
            movingPieceType = PAWN_CODE;
        else
            assert(false);
    }
    else {
        if (blackKingPosition == endSquare)
            return 0;
        else if (blackPieceTypes[QUEEN_CODE] & endSquare)
            movingPieceType = QUEEN_CODE;
        else if (blackPieceTypes[ROOK_CODE] & endSquare)
            movingPieceType = ROOK_CODE;
        else if (blackPieceTypes[BISHOP_CODE] & endSquare)
            movingPieceType = BISHOP_CODE;
        else if (blackPieceTypes[KNIGHT_CODE] & endSquare)
            movingPieceType = KNIGHT_CODE;
        else if (blackPieceTypes[PAWN_CODE] & endSquare)
            movingPieceType = PAWN_CODE;
        else
            assert(false);
    }

    return getQuietSEE(movingPieceType,move);
} // end getQuietSEE

void ChessBoard::getNonnegativeSEECapturesOnly (vector<move_t>& captures) const {
    const bitboard_t diagonalSquaresFromKing = isItWhiteToMove ? getEmptyBoardMagicBishopAttackedSquares(whiteKingPosition) : getEmptyBoardMagicBishopAttackedSquares(blackKingPosition);
    const bitboard_t orthogonalSquaresFromKing = isItWhiteToMove ? getEmptyBoardMagicRookAttackedSquares(whiteKingPosition) : getEmptyBoardMagicRookAttackedSquares(blackKingPosition);
    const bitboard_t effectiveEnemyBishops = isItWhiteToMove ? diagonalSquaresFromKing & (blackPieceTypes[BISHOP_CODE] | blackPieceTypes[QUEEN_CODE]) : diagonalSquaresFromKing & (whitePieceTypes[BISHOP_CODE] | whitePieceTypes[QUEEN_CODE]);
    const bitboard_t effectiveEnemyRooks = isItWhiteToMove ? orthogonalSquaresFromKing & (blackPieceTypes[ROOK_CODE] | blackPieceTypes[QUEEN_CODE]) : orthogonalSquaresFromKing & (whitePieceTypes[ROOK_CODE] | whitePieceTypes[QUEEN_CODE]);
    const bitboard_t enemyAttackedSquares = isItWhiteToMove ? calculateBlackAttackedSquares() : calculateWhiteAttackedSquares();
    const uint8_t myKingPosition = isItWhiteToMove ? whiteKingPosition : blackKingPosition;
    const bitboard_t myPieces = isItWhiteToMove ? allWhitePieces : allBlackPieces;
    const bitboard_t enemyPieces = isItWhiteToMove ? allBlackPieces : allWhitePieces;
    const bitboard_t allPieces = allWhitePieces | allBlackPieces;

    bitboard_t bishopPinnedPieces = 0ULL;
    bitboard_t rookPinnedPieces = 0ULL;

    bitboard_t thisPinningPieceMask, interposingSquares, interposingOccupiedSquares;
    int thisPinningPieceSquare;
    bitboard_t effectiveEnemyBishops1 = effectiveEnemyBishops, effectiveEnemyRooks1 = effectiveEnemyRooks;
    while (effectiveEnemyBishops1 != 0ULL) {
        thisPinningPieceMask = effectiveEnemyBishops1 & -effectiveEnemyBishops1;
        effectiveEnemyBishops1 -= thisPinningPieceMask;
        thisPinningPieceSquare = log2ll(thisPinningPieceMask);
        interposingSquares = lookupCheckResponses(myKingPosition,thisPinningPieceSquare) - (1ULL << thisPinningPieceSquare);
        interposingOccupiedSquares = interposingSquares & allPieces;
        if ((1ULL << log2ll(interposingOccupiedSquares)) == interposingOccupiedSquares)
            bishopPinnedPieces |= interposingSquares;
        // In other words, if there is only one piece between the Bishop and the King, then it's pinned
        // And we might have caught an opponent's piece that could give a discovered attack if it were their turn, but that won't affect anything.
    } // end while loop
    while (effectiveEnemyRooks1 != 0ULL) {
        thisPinningPieceMask = effectiveEnemyRooks1 & -effectiveEnemyRooks1;
        effectiveEnemyRooks1 -= thisPinningPieceMask;
        thisPinningPieceSquare = log2ll(thisPinningPieceMask);
        interposingSquares = lookupCheckResponses(myKingPosition,thisPinningPieceSquare) - (1ULL << thisPinningPieceSquare);
        interposingOccupiedSquares = interposingSquares & allPieces;
        if ((1ULL << log2ll(interposingOccupiedSquares)) == interposingOccupiedSquares)
            rookPinnedPieces |= interposingSquares;
        // In other words, if there is only one piece between the Rook and the King, then it's pinned
        // And we might have caught an opponent's piece that could give a discovered attack if it were their turn, but that won't affect anything.
    } // end while loop

    const bitboard_t kingLegalEndSquares = getMagicKingAttackedSquares(myKingPosition) & ~enemyAttackedSquares & enemyPieces;
    addLegalKingMoves(captures,kingLegalEndSquares);
    addEnPassant(captures,effectiveEnemyBishops,effectiveEnemyRooks);
    if (pieceGivingCheck == DOUBLE_CHECK_CODE)
        return;

    bitboard_t piecesRemaining;
    bitboard_t startSquareMask;
    int startSquare;
    bitboard_t legalEndSquares;
    bitboard_t endSquareMask;
    int endSquare;

    const bitboard_t legalCheckBlockSquares = pieceGivingCheck == NOT_IN_CHECK_CODE ? ENTIRE_BOARD : lookupCheckResponses(myKingPosition,this->pieceGivingCheck);

    const bitboard_t myPawns = isItWhiteToMove ? whitePieceTypes[PAWN_CODE] : blackPieceTypes[PAWN_CODE];
    const int leftCaptureOffset = isItWhiteToMove ? 7 : 9;
    const int rightCaptureOffset = isItWhiteToMove ? 9 : 7; // but we are right shifting instead of left shifting

    // Pawns that can left capture
    piecesRemaining = myPawns & (enemyPieces & legalCheckBlockSquares) << leftCaptureOffset & ~rookPinnedPieces & (~bishopPinnedPieces | effectiveEnemyBishops << leftCaptureOffset);
    while (piecesRemaining != 0) {
        startSquareMask = piecesRemaining & -piecesRemaining;
        piecesRemaining -= startSquareMask;
        startSquare = log2ll(startSquareMask);
        endSquare = startSquare - leftCaptureOffset;
        if ((endSquare & 7) == 7 or (endSquare & 7) == 0) {
            captures.push_back(startSquare << 10 | endSquare << 4 | PROMOTE_TO_QUEEN_FLAG);
        }
        else {
            captures.push_back(getCaptureMove(startSquare,endSquare));
        }
    } // end left captures

    // Pawns that can right capture
    piecesRemaining = myPawns & (enemyPieces & legalCheckBlockSquares) >> rightCaptureOffset & ~rookPinnedPieces & (~bishopPinnedPieces | effectiveEnemyBishops >> rightCaptureOffset);
    while (piecesRemaining != 0) {
        startSquareMask = piecesRemaining & -piecesRemaining;
        piecesRemaining -= startSquareMask;
        startSquare = log2ll(startSquareMask);
        endSquare = startSquare + rightCaptureOffset;
        if ((endSquare & 7) == 7 or (endSquare & 7) == 0) {
            captures.push_back(startSquare << 10 | endSquare << 4 | PROMOTE_TO_QUEEN_FLAG);
        }
        else {
            captures.push_back(getCaptureMove(startSquare,endSquare));
        }
    } // end right captures

    move_t captureMove;

    // Loop over all piece types, except pawns.
    for (int pieceType = KNIGHT_CODE; pieceType >= QUEEN_CODE; pieceType--) {
        piecesRemaining = isItWhiteToMove ? whitePieceTypes[pieceType] : blackPieceTypes[pieceType];
        while (piecesRemaining != 0ULL) {
            startSquareMask = piecesRemaining & -piecesRemaining;
            piecesRemaining -= startSquareMask;
            startSquare = log2ll(startSquareMask);
            legalEndSquares = getMagicWhiteAttackedSquares(pieceType, startSquare, allPieces) & legalCheckBlockSquares & enemyPieces;
            if (bishopPinnedPieces & startSquareMask)
                legalEndSquares &= diagonalSquaresFromKing & getEmptyBoardMagicBishopAttackedSquares(startSquare);
            else if (rookPinnedPieces & startSquareMask)
                legalEndSquares &= orthogonalSquaresFromKing & getEmptyBoardMagicRookAttackedSquares(startSquare);

            while (legalEndSquares != 0ULL) {
                endSquareMask = legalEndSquares & -legalEndSquares;
                legalEndSquares -= endSquareMask;
                endSquare = log2ll(endSquareMask);
                captureMove = getCaptureMove(startSquare, endSquare);
                if ((enemyAttackedSquares & endSquareMask) == 0 or getCaptureSEE(pieceType,captureMove) >= 0)
                    captures.push_back(captureMove);
            } // end while legalEndSquares is not 0
        } // end while piecesRemaining is not 0
    } // end for loop over all piece types
} // end getNonnegativeSEECapturesOnly

void ChessBoard::getLegalMoves (vector<move_t>& winningEqualCaptures, vector<move_t>& losingCaptures, vector<move_t>& nonCaptures) const {
    const bitboard_t diagonalSquaresFromKing = isItWhiteToMove ? getEmptyBoardMagicBishopAttackedSquares(whiteKingPosition) : getEmptyBoardMagicBishopAttackedSquares(blackKingPosition);
    const bitboard_t orthogonalSquaresFromKing = isItWhiteToMove ? getEmptyBoardMagicRookAttackedSquares(whiteKingPosition) : getEmptyBoardMagicRookAttackedSquares(blackKingPosition);
    const bitboard_t effectiveEnemyBishops = isItWhiteToMove ? diagonalSquaresFromKing & (blackPieceTypes[BISHOP_CODE] | blackPieceTypes[QUEEN_CODE]) : diagonalSquaresFromKing & (whitePieceTypes[BISHOP_CODE] | whitePieceTypes[QUEEN_CODE]);
    const bitboard_t effectiveEnemyRooks = isItWhiteToMove ? orthogonalSquaresFromKing & (blackPieceTypes[ROOK_CODE] | blackPieceTypes[QUEEN_CODE]) : orthogonalSquaresFromKing & (whitePieceTypes[ROOK_CODE] | whitePieceTypes[QUEEN_CODE]);
    const bitboard_t enemyAttackedSquares = isItWhiteToMove ? calculateBlackAttackedSquares() : calculateWhiteAttackedSquares();
    const uint8_t myKingPosition = isItWhiteToMove ? whiteKingPosition : blackKingPosition;
    const bitboard_t myPieces = isItWhiteToMove ? allWhitePieces : allBlackPieces;
    const bitboard_t enemyPieces = isItWhiteToMove ? allBlackPieces : allWhitePieces;
    const bitboard_t allPieces = allWhitePieces | allBlackPieces;

    bitboard_t bishopPinnedPieces = 0ULL;
    bitboard_t rookPinnedPieces = 0ULL;

    bitboard_t thisPinningPieceMask, interposingSquares, interposingOccupiedSquares;
    int thisPinningPieceSquare;
    bitboard_t effectiveEnemyBishops1 = effectiveEnemyBishops, effectiveEnemyRooks1 = effectiveEnemyRooks;
    while (effectiveEnemyBishops1 != 0ULL) {
        thisPinningPieceMask = effectiveEnemyBishops1 & -effectiveEnemyBishops1;
        effectiveEnemyBishops1 -= thisPinningPieceMask;
        thisPinningPieceSquare = log2ll(thisPinningPieceMask);
        interposingSquares = lookupCheckResponses(myKingPosition,thisPinningPieceSquare) - (1ULL << thisPinningPieceSquare);
        interposingOccupiedSquares = interposingSquares & allPieces;
        if ((1ULL << log2ll(interposingOccupiedSquares)) == interposingOccupiedSquares)
            bishopPinnedPieces |= interposingSquares;
        // In other words, if there is only one piece between the Bishop and the King, then it's pinned
        // And we might have caught an opponent's piece that could give a discovered attack if it were their turn, but that won't affect anything.
    } // end while loop
    while (effectiveEnemyRooks1 != 0ULL) {
        thisPinningPieceMask = effectiveEnemyRooks1 & -effectiveEnemyRooks1;
        effectiveEnemyRooks1 -= thisPinningPieceMask;
        thisPinningPieceSquare = log2ll(thisPinningPieceMask);
        interposingSquares = lookupCheckResponses(myKingPosition,thisPinningPieceSquare) - (1ULL << thisPinningPieceSquare);
        interposingOccupiedSquares = interposingSquares & allPieces;
        if ((1ULL << log2ll(interposingOccupiedSquares)) == interposingOccupiedSquares)
            rookPinnedPieces |= interposingSquares;
        // In other words, if there is only one piece between the Rook and the King, then it's pinned
        // And we might have caught an opponent's piece that could give a discovered attack if it were their turn, but that won't affect anything.
    } // end while loop

    const bitboard_t kingLegalEndSquares = getMagicKingAttackedSquares(myKingPosition) & ~enemyAttackedSquares & ~myPieces;
    const bitboard_t kingCaptureEndSquares = kingLegalEndSquares & enemyPieces;
    const bitboard_t kingQuietEndSquares = kingLegalEndSquares - kingCaptureEndSquares;
    addLegalKingMoves(winningEqualCaptures,kingCaptureEndSquares);
    addLegalKingMoves(nonCaptures,kingQuietEndSquares);
    if (pieceGivingCheck == DOUBLE_CHECK_CODE)
        return;

    bitboard_t legalCheckBlockSquares = ENTIRE_BOARD;
    addEnPassant(winningEqualCaptures,effectiveEnemyBishops,effectiveEnemyRooks);
    if (pieceGivingCheck == NOT_IN_CHECK_CODE)
        addCastling(nonCaptures,enemyAttackedSquares);
    else
        legalCheckBlockSquares = lookupCheckResponses(myKingPosition,this->pieceGivingCheck);

    bitboard_t piecesRemaining;
    bitboard_t startSquareMask;
    int startSquare;
    bitboard_t legalEndSquares;
    bitboard_t endSquareMask;
    int endSquare;
    move_t captureMove;

    const bitboard_t myPawns = isItWhiteToMove ? whitePieceTypes[PAWN_CODE] : blackPieceTypes[PAWN_CODE];
    const int leftCaptureOffset = isItWhiteToMove ? 7 : 9;
    const int rightCaptureOffset = isItWhiteToMove ? 9 : 7; // but we are right shifting instead of left shifting
    const int pawnSinglePushOffset = isItWhiteToMove ? 1 : -1;
    const int pawnDoublePushOffset = isItWhiteToMove ? 2 : -2;

    // Pawns that can left capture
    piecesRemaining = myPawns & (enemyPieces & legalCheckBlockSquares) << leftCaptureOffset & ~rookPinnedPieces & (~bishopPinnedPieces | effectiveEnemyBishops << leftCaptureOffset);
    while (piecesRemaining != 0) {
        startSquareMask = piecesRemaining & -piecesRemaining;
        piecesRemaining -= startSquareMask;
        startSquare = log2ll(startSquareMask);
        endSquare = startSquare - leftCaptureOffset;
        if ((endSquare & 7) == 7 or (endSquare & 7) == 0) {
            winningEqualCaptures.push_back(startSquare << 10 | endSquare << 4 | PROMOTE_TO_QUEEN_FLAG);
            losingCaptures.push_back(startSquare << 10 | endSquare << 4 | PROMOTE_TO_ROOK_FLAG);
            losingCaptures.push_back(startSquare << 10 | endSquare << 4 | PROMOTE_TO_BISHOP_FLAG);
            losingCaptures.push_back(startSquare << 10 | endSquare << 4 | PROMOTE_TO_KNIGHT_FLAG);
        }
        else {
            winningEqualCaptures.push_back(getCaptureMove(startSquare,endSquare));
        }
    } // end left captures

    // Pawns that can right capture
    piecesRemaining = myPawns & (enemyPieces & legalCheckBlockSquares) >> rightCaptureOffset & ~rookPinnedPieces & (~bishopPinnedPieces | effectiveEnemyBishops >> rightCaptureOffset);
    while (piecesRemaining != 0) {
        startSquareMask = piecesRemaining & -piecesRemaining;
        piecesRemaining -= startSquareMask;
        startSquare = log2ll(startSquareMask);
        endSquare = startSquare + rightCaptureOffset;
        if ((endSquare & 7) == 7 or (endSquare & 7) == 0) {
            winningEqualCaptures.push_back(startSquare << 10 | endSquare << 4 | PROMOTE_TO_QUEEN_FLAG);
            losingCaptures.push_back(startSquare << 10 | endSquare << 4 | PROMOTE_TO_ROOK_FLAG);
            losingCaptures.push_back(startSquare << 10 | endSquare << 4 | PROMOTE_TO_BISHOP_FLAG);
            losingCaptures.push_back(startSquare << 10 | endSquare << 4 | PROMOTE_TO_KNIGHT_FLAG);
        }
        else {
            winningEqualCaptures.push_back(getCaptureMove(startSquare,endSquare));
        }
    } // end right captures

    // Loop over all piece types, except pawns.
    for (int pieceType = KNIGHT_CODE; pieceType >= QUEEN_CODE; pieceType--) {
        piecesRemaining = isItWhiteToMove ? whitePieceTypes[pieceType] : blackPieceTypes[pieceType];
        while (piecesRemaining != 0ULL) {
            startSquareMask = piecesRemaining & -piecesRemaining;
            piecesRemaining -= startSquareMask;
            startSquare = log2ll(startSquareMask);
            legalEndSquares = getMagicWhiteAttackedSquares(pieceType, startSquare, allPieces) & legalCheckBlockSquares & ~myPieces;
            if (bishopPinnedPieces & startSquareMask)
                legalEndSquares &= diagonalSquaresFromKing & getEmptyBoardMagicBishopAttackedSquares(startSquare);
            else if (rookPinnedPieces & startSquareMask)
                legalEndSquares &= orthogonalSquaresFromKing & getEmptyBoardMagicRookAttackedSquares(startSquare);

            while (legalEndSquares != 0ULL) {
                endSquareMask = legalEndSquares & -legalEndSquares;
                legalEndSquares -= endSquareMask;
                endSquare = log2ll(endSquareMask);
                if (enemyPieces & endSquareMask) {
                    captureMove = getCaptureMove(startSquare, endSquare);
                    if ((enemyAttackedSquares & endSquareMask) == 0 or getCaptureSEE(pieceType,captureMove) >= 0)
                        winningEqualCaptures.push_back(captureMove);
                    else
                        losingCaptures.push_back(captureMove);
                }
                else {
                    nonCaptures.push_back(startSquare << 10 | endSquare << 4 | NORMAL_MOVE_FLAG);
                }
            } // end while legalEndSquares is not 0
        } // end while piecesRemaining is not 0
    } // end for loop over all piece types

    // Pawn pushes two squares
    piecesRemaining = isItWhiteToMove ?
                      myPawns & SECOND_RANK & ~allPieces >> 1 & ~allPieces >> 2 & legalCheckBlockSquares >> 2 & ~bishopPinnedPieces & (~rookPinnedPieces | 255ULL << (myKingPosition & 56)) :
                      myPawns &SEVENTH_RANK & ~allPieces << 1 & ~allPieces << 2 & legalCheckBlockSquares << 2 & ~bishopPinnedPieces & (~rookPinnedPieces | 255ULL << (myKingPosition & 56));

    while (piecesRemaining != 0ULL) {
        startSquareMask = piecesRemaining & -piecesRemaining;
        piecesRemaining -= startSquareMask;
        startSquare = log2ll(startSquareMask);
        endSquare = startSquare + pawnDoublePushOffset;
        nonCaptures.push_back(startSquare << 10 | endSquare << 4 | PAWN_PUSH_TWO_SQUARES_FLAG);
    } // end pawn pushes two squares

    // Pawn pushes one square
    piecesRemaining = isItWhiteToMove ?
                      myPawns & (~allPieces & legalCheckBlockSquares) >> 1 & ~bishopPinnedPieces & (~rookPinnedPieces | 255ULL << (myKingPosition & 56)) :
                      myPawns & (~allPieces & legalCheckBlockSquares) << 1 & ~bishopPinnedPieces & (~rookPinnedPieces | 255ULL << (myKingPosition & 56));

    while (piecesRemaining != 0ULL) {
        startSquareMask = piecesRemaining & -piecesRemaining;
        piecesRemaining -= startSquareMask;
        startSquare = log2ll(startSquareMask);
        endSquare = startSquare + pawnSinglePushOffset;
        if ((endSquare & 7) == 7 or (endSquare & 7) == 0) {
            nonCaptures.push_back(startSquare << 10 | endSquare << 4 | PROMOTE_TO_QUEEN_FLAG);
            nonCaptures.push_back(startSquare << 10 | endSquare << 4 | PROMOTE_TO_ROOK_FLAG);
            nonCaptures.push_back(startSquare << 10 | endSquare << 4 | PROMOTE_TO_BISHOP_FLAG);
            nonCaptures.push_back(startSquare << 10 | endSquare << 4 | PROMOTE_TO_KNIGHT_FLAG);
        }
        else {
            nonCaptures.push_back(startSquare << 10 | endSquare << 4 | NORMAL_MOVE_FLAG);
        }
    } // end pawn pushes one square
}

void ChessBoard::getLegalMoves (vector<move_t>& winningEqualCaptures, vector<move_t>& losingCaptures, vector<move_t>& zeroSEEQuiets, vector<move_t>& negativeSEEQuiets) const {
    const bitboard_t diagonalSquaresFromKing = isItWhiteToMove ? getEmptyBoardMagicBishopAttackedSquares(whiteKingPosition) : getEmptyBoardMagicBishopAttackedSquares(blackKingPosition);
    const bitboard_t orthogonalSquaresFromKing = isItWhiteToMove ? getEmptyBoardMagicRookAttackedSquares(whiteKingPosition) : getEmptyBoardMagicRookAttackedSquares(blackKingPosition);
    const bitboard_t effectiveEnemyBishops = isItWhiteToMove ? diagonalSquaresFromKing & (blackPieceTypes[BISHOP_CODE] | blackPieceTypes[QUEEN_CODE]) : diagonalSquaresFromKing & (whitePieceTypes[BISHOP_CODE] | whitePieceTypes[QUEEN_CODE]);
    const bitboard_t effectiveEnemyRooks = isItWhiteToMove ? orthogonalSquaresFromKing & (blackPieceTypes[ROOK_CODE] | blackPieceTypes[QUEEN_CODE]) : orthogonalSquaresFromKing & (whitePieceTypes[ROOK_CODE] | whitePieceTypes[QUEEN_CODE]);
    const bitboard_t enemyAttackedSquares = isItWhiteToMove ? calculateBlackAttackedSquares() : calculateWhiteAttackedSquares();
    const uint8_t myKingPosition = isItWhiteToMove ? whiteKingPosition : blackKingPosition;
    const bitboard_t myPieces = isItWhiteToMove ? allWhitePieces : allBlackPieces;
    const bitboard_t enemyPieces = isItWhiteToMove ? allBlackPieces : allWhitePieces;
    const bitboard_t allPieces = allWhitePieces | allBlackPieces;

    bitboard_t bishopPinnedPieces = 0ULL;
    bitboard_t rookPinnedPieces = 0ULL;

    bitboard_t thisPinningPieceMask, interposingSquares, interposingOccupiedSquares;
    int thisPinningPieceSquare;
    bitboard_t effectiveEnemyBishops1 = effectiveEnemyBishops, effectiveEnemyRooks1 = effectiveEnemyRooks;
    while (effectiveEnemyBishops1 != 0ULL) {
        thisPinningPieceMask = effectiveEnemyBishops1 & -effectiveEnemyBishops1;
        effectiveEnemyBishops1 -= thisPinningPieceMask;
        thisPinningPieceSquare = log2ll(thisPinningPieceMask);
        interposingSquares = lookupCheckResponses(myKingPosition,thisPinningPieceSquare) - (1ULL << thisPinningPieceSquare);
        interposingOccupiedSquares = interposingSquares & allPieces;
        if ((1ULL << log2ll(interposingOccupiedSquares)) == interposingOccupiedSquares)
            bishopPinnedPieces |= interposingSquares;
        // In other words, if there is only one piece between the Bishop and the King, then it's pinned
        // And we might have caught an opponent's piece that could give a discovered attack if it were their turn, but that won't affect anything.
    } // end while loop
    while (effectiveEnemyRooks1 != 0ULL) {
        thisPinningPieceMask = effectiveEnemyRooks1 & -effectiveEnemyRooks1;
        effectiveEnemyRooks1 -= thisPinningPieceMask;
        thisPinningPieceSquare = log2ll(thisPinningPieceMask);
        interposingSquares = lookupCheckResponses(myKingPosition,thisPinningPieceSquare) - (1ULL << thisPinningPieceSquare);
        interposingOccupiedSquares = interposingSquares & allPieces;
        if ((1ULL << log2ll(interposingOccupiedSquares)) == interposingOccupiedSquares)
            rookPinnedPieces |= interposingSquares;
        // In other words, if there is only one piece between the Rook and the King, then it's pinned
        // And we might have caught an opponent's piece that could give a discovered attack if it were their turn, but that won't affect anything.
    } // end while loop

    const bitboard_t kingLegalEndSquares = getMagicKingAttackedSquares(myKingPosition) & ~enemyAttackedSquares & ~myPieces;
    const bitboard_t kingCaptureEndSquares = kingLegalEndSquares & enemyPieces;
    const bitboard_t kingQuietEndSquares = kingLegalEndSquares - kingCaptureEndSquares;
    addLegalKingMoves(winningEqualCaptures,kingCaptureEndSquares);
    addLegalKingMoves(zeroSEEQuiets,kingQuietEndSquares);
    if (pieceGivingCheck == DOUBLE_CHECK_CODE)
        return;

    bitboard_t legalCheckBlockSquares = ENTIRE_BOARD;
    addEnPassant(winningEqualCaptures,effectiveEnemyBishops,effectiveEnemyRooks);
    if (pieceGivingCheck == NOT_IN_CHECK_CODE)
        addCastling(zeroSEEQuiets,enemyAttackedSquares);
    else
        legalCheckBlockSquares = lookupCheckResponses(myKingPosition,this->pieceGivingCheck);

    bitboard_t piecesRemaining;
    bitboard_t startSquareMask;
    int startSquare;
    bitboard_t legalEndSquares;
    bitboard_t endSquareMask;
    int endSquare;
    move_t move;

    const bitboard_t myPawns = isItWhiteToMove ? whitePieceTypes[PAWN_CODE] : blackPieceTypes[PAWN_CODE];
    const int leftCaptureOffset = isItWhiteToMove ? 7 : 9;
    const int rightCaptureOffset = isItWhiteToMove ? 9 : 7; // but we are right shifting instead of left shifting
    const int pawnSinglePushOffset = isItWhiteToMove ? 1 : -1;
    const int pawnDoublePushOffset = isItWhiteToMove ? 2 : -2;

    // Pawns that can left capture
    piecesRemaining = myPawns & (enemyPieces & legalCheckBlockSquares) << leftCaptureOffset & ~rookPinnedPieces & (~bishopPinnedPieces | effectiveEnemyBishops << leftCaptureOffset);
    while (piecesRemaining != 0) {
        startSquareMask = piecesRemaining & -piecesRemaining;
        piecesRemaining -= startSquareMask;
        startSquare = log2ll(startSquareMask);
        endSquare = startSquare - leftCaptureOffset;
        if ((endSquare & 7) == 7 or (endSquare & 7) == 0) {
            winningEqualCaptures.push_back(startSquare << 10 | endSquare << 4 | PROMOTE_TO_QUEEN_FLAG);
            losingCaptures.push_back(startSquare << 10 | endSquare << 4 | PROMOTE_TO_ROOK_FLAG);
            losingCaptures.push_back(startSquare << 10 | endSquare << 4 | PROMOTE_TO_BISHOP_FLAG);
            losingCaptures.push_back(startSquare << 10 | endSquare << 4 | PROMOTE_TO_KNIGHT_FLAG);
        }
        else {
            winningEqualCaptures.push_back(getCaptureMove(startSquare,endSquare));
        }
    } // end left captures

    // Pawns that can right capture
    piecesRemaining = myPawns & (enemyPieces & legalCheckBlockSquares) >> rightCaptureOffset & ~rookPinnedPieces & (~bishopPinnedPieces | effectiveEnemyBishops >> rightCaptureOffset);
    while (piecesRemaining != 0) {
        startSquareMask = piecesRemaining & -piecesRemaining;
        piecesRemaining -= startSquareMask;
        startSquare = log2ll(startSquareMask);
        endSquare = startSquare + rightCaptureOffset;
        if ((endSquare & 7) == 7 or (endSquare & 7) == 0) {
            winningEqualCaptures.push_back(startSquare << 10 | endSquare << 4 | PROMOTE_TO_QUEEN_FLAG);
            losingCaptures.push_back(startSquare << 10 | endSquare << 4 | PROMOTE_TO_ROOK_FLAG);
            losingCaptures.push_back(startSquare << 10 | endSquare << 4 | PROMOTE_TO_BISHOP_FLAG);
            losingCaptures.push_back(startSquare << 10 | endSquare << 4 | PROMOTE_TO_KNIGHT_FLAG);
        }
        else {
            winningEqualCaptures.push_back(getCaptureMove(startSquare,endSquare));
        }
    } // end right captures

    // Loop over all piece types, except pawns.
    for (int pieceType = KNIGHT_CODE; pieceType >= QUEEN_CODE; pieceType--) {
        piecesRemaining = isItWhiteToMove ? whitePieceTypes[pieceType] : blackPieceTypes[pieceType];
        while (piecesRemaining != 0ULL) {
            startSquareMask = piecesRemaining & -piecesRemaining;
            piecesRemaining -= startSquareMask;
            startSquare = log2ll(startSquareMask);
            legalEndSquares = getMagicWhiteAttackedSquares(pieceType, startSquare, allPieces) & legalCheckBlockSquares & ~myPieces;
            if (bishopPinnedPieces & startSquareMask)
                legalEndSquares &= diagonalSquaresFromKing & getEmptyBoardMagicBishopAttackedSquares(startSquare);
            else if (rookPinnedPieces & startSquareMask)
                legalEndSquares &= orthogonalSquaresFromKing & getEmptyBoardMagicRookAttackedSquares(startSquare);

            while (legalEndSquares != 0ULL) {
                endSquareMask = legalEndSquares & -legalEndSquares;
                legalEndSquares -= endSquareMask;
                endSquare = log2ll(endSquareMask);
                if (enemyPieces & endSquareMask) {
                    move = getCaptureMove(startSquare, endSquare);
                    if ((enemyAttackedSquares & endSquareMask) == 0 or getCaptureSEE(pieceType,move) >= 0)
                        winningEqualCaptures.push_back(move);
                    else
                        losingCaptures.push_back(move);
                }
                else {
                    move = startSquare << 10 | endSquare << 4 | NORMAL_MOVE_FLAG;
                    if (getQuietSEE(pieceType,move) >= 0)
                        zeroSEEQuiets.push_back(move);
                    else
                        negativeSEEQuiets.push_back(move);
                }
            } // end while legalEndSquares is not 0
        } // end while piecesRemaining is not 0
    } // end for loop over all piece types

    // Pawn pushes two squares
    piecesRemaining = isItWhiteToMove ?
                      myPawns & SECOND_RANK & ~allPieces >> 1 & ~allPieces >> 2 & legalCheckBlockSquares >> 2 & ~bishopPinnedPieces & (~rookPinnedPieces | 255ULL << (myKingPosition & 56)) :
                      myPawns &SEVENTH_RANK & ~allPieces << 1 & ~allPieces << 2 & legalCheckBlockSquares << 2 & ~bishopPinnedPieces & (~rookPinnedPieces | 255ULL << (myKingPosition & 56));

    while (piecesRemaining != 0ULL) {
        startSquareMask = piecesRemaining & -piecesRemaining;
        piecesRemaining -= startSquareMask;
        startSquare = log2ll(startSquareMask);
        endSquare = startSquare + pawnDoublePushOffset;
        move = startSquare << 10 | endSquare << 4 | PAWN_PUSH_TWO_SQUARES_FLAG;
        if (getQuietSEE(PAWN_CODE,move) >= 0)
            zeroSEEQuiets.push_back(move);
        else
            negativeSEEQuiets.push_back(move);
    } // end pawn pushes two squares

    // Pawn pushes one square
    piecesRemaining = isItWhiteToMove ?
                      myPawns & (~allPieces & legalCheckBlockSquares) >> 1 & ~bishopPinnedPieces & (~rookPinnedPieces | 255ULL << (myKingPosition & 56)) :
                      myPawns & (~allPieces & legalCheckBlockSquares) << 1 & ~bishopPinnedPieces & (~rookPinnedPieces | 255ULL << (myKingPosition & 56));

    while (piecesRemaining != 0ULL) {
        startSquareMask = piecesRemaining & -piecesRemaining;
        piecesRemaining -= startSquareMask;
        startSquare = log2ll(startSquareMask);
        endSquare = startSquare + pawnSinglePushOffset;
        if ((endSquare & 7) == 7 or (endSquare & 7) == 0) {
            move = startSquare << 10 | endSquare << 4 | PROMOTE_TO_QUEEN_FLAG;
            if (getQuietSEE(PAWN_CODE,move) >= 0)
                zeroSEEQuiets.push_back(move);
            else
                negativeSEEQuiets.push_back(move);
            negativeSEEQuiets.push_back(startSquare << 10 | endSquare << 4 | PROMOTE_TO_ROOK_FLAG);
            negativeSEEQuiets.push_back(startSquare << 10 | endSquare << 4 | PROMOTE_TO_BISHOP_FLAG);
            negativeSEEQuiets.push_back(startSquare << 10 | endSquare << 4 | PROMOTE_TO_KNIGHT_FLAG);
        }
        else {
            move = startSquare << 10 | endSquare << 4 | NORMAL_MOVE_FLAG;
            if (getQuietSEE(PAWN_CODE,move) >= 0)
                zeroSEEQuiets.push_back(move);
            else
                negativeSEEQuiets.push_back(move);
        }
    } // end pawn pushes one square
} // end getLegalMoves that passes in four vectors by reference.

int ChessBoard::perft (const int depth) const {
    if (depth == 0)
        return 1;

    MoveList legalMoves;
    getLegalMoves1(legalMoves);

    int nodeCount = 0;
    for (int i = 0; i < legalMoves.size; i++) {
        ChessBoard copy = *this;
        copy.makemove(legalMoves.moveList[i]);
        nodeCount += copy.perft(depth - 1);
    }
    return nodeCount;
}

int ChessBoard::capturePerft (const int depth) const {
    if (depth == 1) {
        vector<move_t> captures;
        captures.reserve(16);
        getLegalCapturesOnly(captures);
        return captures.size();
    }

    vector<move_t> legalMoves;
    legalMoves.reserve(52);
    getLegalMoves(legalMoves);

    int nodeCount = 0;
    for (move_t move : legalMoves) {
        ChessBoard copy = *this;
        copy.makemove(move);
        nodeCount += copy.capturePerft(depth - 1);
    }
    return nodeCount;
}

eval_t ChessBoard::getStaticEval () const {
    using namespace hce;
    if (whiteWonByCheckmate)
        return MATE_VALUE;
    else if (blackWonByCheckmate)
        return -MATE_VALUE;
    else if (drawByInsufficientMaterial or drawByStalemate)
        return 0;

    // king PSTs
    packed_eval_t packedScore = king_psts[whiteKingPosition] - king_psts[blackKingPosition ^ 7];
    int phase = 0; // 24 = mg, 0 = mg
    bitboard_t piecesRemaining;
    bitboard_t thisPieceMask;
    int thisPieceSquare;
    for (int pieceType = QUEEN_CODE; pieceType <= PAWN_CODE; pieceType++) {
        // phase transition
        phase += __builtin_popcountll(whitePieceTypes[pieceType] | blackPieceTypes[pieceType]) * PHASE_PIECE_VALUES[pieceType];
        
        // white PSTs
        piecesRemaining = whitePieceTypes[pieceType];
        while (piecesRemaining != 0ULL) {
            thisPieceMask = piecesRemaining & -piecesRemaining;
            piecesRemaining -= thisPieceMask;
            thisPieceSquare = log2ll(thisPieceMask);
            packedScore += piece_type_psts[pieceType][thisPieceSquare];
        }
        
        // black PSTs
        piecesRemaining = blackPieceTypes[pieceType];
        while (piecesRemaining != 0ULL) {
            thisPieceMask = piecesRemaining & -piecesRemaining;
            piecesRemaining -= thisPieceMask;
            thisPieceSquare = log2ll(thisPieceMask);
            packedScore -= piece_type_psts[pieceType][thisPieceSquare ^ 7];
        }
    }
    
    return getEvalFromPacked(packedScore,phase);
} // end getStaticEval

eval_t ChessBoard::getNegaStaticEval () const {
    if (isItWhiteToMove)
        return getStaticEval();
    else
        return -getStaticEval();
}

bool ChessBoard::canMakeNullMove () const {
    if (isItWhiteToMove)
        return pieceGivingCheck == NOT_IN_CHECK_CODE and allWhitePieces != (1ULL << whiteKingPosition) + whitePieceTypes[PAWN_CODE];
    else
        return pieceGivingCheck == NOT_IN_CHECK_CODE and allBlackPieces != (1ULL << blackKingPosition) + blackPieceTypes[PAWN_CODE];
}

void ChessBoard::makeNullMove () {
    isItWhiteToMove = !isItWhiteToMove;
    if (whichPawnMovedTwoSquares < 8) {
        zobristCode ^= zobrist::WHICH_PAWN_MOVED_TWO_SQUARES_CODES[whichPawnMovedTwoSquares] ^
                       zobrist::WHICH_PAWN_MOVED_TWO_SQUARES_CODES[255];
        whichPawnMovedTwoSquares = 255;
    }
    zobristCode ^= zobrist::IS_IT_WHITE_TO_MOVE_CODE;
    updateMates();
}

void ChessBoard::decomposeMove(const move_t move, Piece& piece, bool& isCapture, int& startSquare, int& endSquare, move_t& flag) const {
    if (isCastle(move)) {
        piece = KING;
        isCapture = false;
        startSquare = move >> 10;
        endSquare = (move >> 4) & 63;
        flag = getFlag(move);
        return;
    }
    startSquare = move >> 10;
    endSquare = (move >> 4) & 63;
    flag = getFlag(move);

    if (isItWhiteToMove)
        isCapture = ((allBlackPieces >> endSquare) & 1ULL) == 1ULL;
    else
        isCapture = ((allWhitePieces >> endSquare) & 1ULL) == 1ULL;

    if (isItWhiteToMove) {
        if (whiteKingPosition == startSquare)
            piece = KING;
        else if (((whitePieceTypes[QUEEN_CODE] >> startSquare) & 1ULL) == 1ULL)
            piece = QUEEN;
        else if (((whitePieceTypes[ROOK_CODE] >> startSquare) & 1ULL) == 1ULL)
            piece = ROOK;
        else if (((whitePieceTypes[BISHOP_CODE] >> startSquare) & 1ULL) == 1ULL)
            piece = BISHOP;
        else if (((whitePieceTypes[KNIGHT_CODE] >> startSquare) & 1ULL) == 1ULL)
            piece = KNIGHT;
        else if (((whitePieceTypes[PAWN_CODE] >> startSquare) & 1ULL) == 1ULL)
            piece = PAWN;
        else {
            cout << toString() << endl;
            cout << "FEN string is " << toFenNotation();
            cout << "tried to convert move " << move << " to algebraic notation. No piece was found on the start square";
            assert(false);
        }
    }

    else {
        if (blackKingPosition == startSquare)
            piece = KING;
        else if (((blackPieceTypes[QUEEN_CODE] >> startSquare) & 1ULL) == 1ULL)
            piece = QUEEN;
        else if (((blackPieceTypes[ROOK_CODE] >> startSquare) & 1ULL) == 1ULL)
            piece = ROOK;
        else if (((blackPieceTypes[BISHOP_CODE] >> startSquare) & 1ULL) == 1ULL)
            piece = BISHOP;
        else if (((blackPieceTypes[KNIGHT_CODE] >> startSquare) & 1ULL) == 1ULL)
            piece = KNIGHT;
        else if (((blackPieceTypes[PAWN_CODE] >> startSquare) & 1ULL) == 1ULL)
            piece = PAWN;
        else {
            cout << toString() << endl;
            cout << "FEN string is " << toFenNotation();
            cout << "tried to convert move " << move << " to algebraic notation. No piece was found on the start square";
            assert(false);
        }
    }
} // end decomposeMove

string ChessBoard::moveToSAN (const move_t move) const {
    if (move == WHITE_SHORT_CASTLE or move == BLACK_SHORT_CASTLE)
        return "O-O";
    if (move == WHITE_LONG_CASTLE or move == BLACK_LONG_CASTLE)
        return "O-O-O";

    bool isCapture;
    Piece piece;
    int startSquare;
    int endSquare;
    u_int16_t flag;
    // We get this by passing everything in by reference to decomposeMove
    decomposeMove(move,piece,isCapture,startSquare,endSquare,flag);

    bool other_isCapture; // this is snamel case (cross of snake and camel). I don't see what's bad about that.
    Piece other_piece;
    int other_startSquare;
    int other_endSquare;
    u_int16_t other_flag;

    // The hard part: Find which other pieces could move here (There is more than one Rd1)
    bool mustSpecifyRank = false;
    bool mustSpecifyFile = false;
    bool mustSpecifySomething = false;

    vector<move_t> legalMoves;
    getLegalMoves(legalMoves);
    for (const move_t otherMove: legalMoves) {
        if (isCastle(otherMove))
            continue;
        if (otherMove == move)
            continue;
        decomposeMove(otherMove,other_piece,other_isCapture,other_startSquare,other_endSquare,other_flag);

        if (other_piece == piece and other_endSquare == endSquare and other_startSquare != startSquare) {
//                cout << otherMove << endl << move << endl;
            mustSpecifySomething = true;
            if (other_startSquare / 8 == startSquare / 8)
                mustSpecifyRank = true;
            if (other_startSquare % 8 == startSquare % 8)
                mustSpecifyFile = true;
        }
    }

    if (mustSpecifySomething and !mustSpecifyRank and !mustSpecifyFile)
        mustSpecifyFile = true;

    string prefix;
    if (piece == PAWN) {
        if (flag == EN_PASSANT_FLAG)
            isCapture = true;
        mustSpecifySomething = false;
        mustSpecifyRank = false;
        mustSpecifyFile = false;
        if (isCapture)
            prefix = string(1, 'a' + (startSquare / 8));
        else
            prefix = "";
    }
    else {
        if (piece == KING)
            prefix = "K";
        else if (piece == QUEEN)
            prefix = "Q";
        else if (piece == ROOK)
            prefix = "R";
        else if (piece == BISHOP)
            prefix = "B";
        else if (piece == KNIGHT)
            prefix = "N";
        else
            assert(false);

        if (mustSpecifyFile)
            prefix += string(1,'a' + (startSquare / 8));
        if (mustSpecifyRank)
            prefix += string(1,'1' + (startSquare % 8));

    }

    // Now we add "x" if it's a capture

    if (isCapture)
        prefix += "x";

    // Now we add the ending square
    prefix += string(1,'a' + (endSquare / 8));
    prefix += string(1,'1' + (endSquare % 8));

    // Now we add if it's a promotion

    if (flag == PROMOTE_TO_QUEEN_FLAG)
        prefix += "=Q";
    else if (flag == PROMOTE_TO_ROOK_FLAG)
        prefix += "=R";
    else if (flag == PROMOTE_TO_BISHOP_FLAG)
        prefix += "=B";
    else if (flag == PROMOTE_TO_KNIGHT_FLAG)
        prefix += "=N";

    bool isCheck = false;
    bool isCheckmate = false;

    ChessBoard newBoard = *this;
    newBoard.makemove(move); // It had better not be illegal.

    isCheck = newBoard.isInCheck();
    isCheckmate = isCheck and (newBoard.isWhiteWonByCheckmate() or newBoard.isBlackWonByCheckmate());

    if (isCheckmate)
        prefix += "#";
    else if (isCheck)
        prefix += "+";

    return prefix;
}

Piece ChessBoard::getMovingPiece(const move_t move) const {
    int startSquare = move >> 10;
    if (whiteKingPosition == startSquare or blackKingPosition == startSquare or move == WHITE_SHORT_CASTLE or move == BLACK_SHORT_CASTLE or move == WHITE_LONG_CASTLE or move == BLACK_LONG_CASTLE)
        return KING;

    if (isItWhiteToMove) {
        if ((whitePieceTypes[QUEEN_CODE] >> startSquare & 1ULL) == 1ULL)
            return QUEEN;
        if ((whitePieceTypes[ROOK_CODE] >> startSquare & 1ULL) == 1ULL)
            return ROOK;
        if ((whitePieceTypes[BISHOP_CODE] >> startSquare & 1ULL) == 1ULL)
            return BISHOP;
        if ((whitePieceTypes[KNIGHT_CODE] >> startSquare & 1ULL) == 1ULL)
            return KNIGHT;
        if ((whitePieceTypes[PAWN_CODE] >> startSquare & 1ULL) == 1ULL)
            return PAWN;
    }
    else {
        if ((blackPieceTypes[QUEEN_CODE] >> startSquare & 1ULL) == 1ULL)
            return QUEEN;
        if ((blackPieceTypes[ROOK_CODE] >> startSquare & 1ULL) == 1ULL)
            return ROOK;
        if ((blackPieceTypes[BISHOP_CODE] >> startSquare & 1ULL) == 1ULL)
            return BISHOP;
        if ((blackPieceTypes[KNIGHT_CODE] >> startSquare & 1ULL) == 1ULL)
            return KNIGHT;
        if ((blackPieceTypes[PAWN_CODE] >> startSquare & 1ULL) == 1ULL)
            return PAWN;
    }
    assert(false);
    // There are no pieces on the start square
}

move_t ChessBoard::getMoveFromSAN (const string& SAN) const {
    if (SAN.find("O-O-O") != std::string::npos) {
        if (isItWhiteToMove)
            return WHITE_LONG_CASTLE;
        else
            return BLACK_LONG_CASTLE;
    }
    else if (SAN.find("O-O") != std::string::npos) {
        if (isItWhiteToMove)
            return WHITE_SHORT_CASTLE;
        else
            return BLACK_SHORT_CASTLE;
    }

    Piece piece;
    const char pieceInitial = SAN[0];
    switch(pieceInitial) {
        case 'K': piece = KING; break;
        case 'Q' : piece = QUEEN; break;
        case 'R': piece = ROOK; break;
        case 'B': piece = BISHOP; break;
        case 'N': piece = KNIGHT; break;
        case 'a': piece = PAWN; break;
        case 'b': piece = PAWN; break;
        case 'c': piece = PAWN; break;
        case 'd': piece = PAWN; break;
        case 'e': piece = PAWN; break;
        case 'f': piece = PAWN; break;
        case 'g': piece = PAWN; break;
        case 'h': piece = PAWN; break;
        default: cout << pieceInitial << endl; exit(65);
    }

    // Now we find the end square. It is always last, except for +, #, or =Q / =R / =B / =N
    // While we're doing this, we also find if it is a promotion

    Piece promotingPiece = KING; // this is the null piece. It is not used except if the move is actually a promotion

    unsigned int endSquareIndex = SAN.size() - 1;
    if (SAN[endSquareIndex] == '+' or SAN[endSquareIndex] == '#')
        endSquareIndex--;
    if (SAN[endSquareIndex - 1] == '=') {
        switch(SAN[endSquareIndex]) {
            case 'Q': promotingPiece = QUEEN; break;
            case 'R': promotingPiece = ROOK; break;
            case 'B': promotingPiece = BISHOP; break;
            case 'N': promotingPiece = KNIGHT; break;
            default: assert(false);
        }
        endSquareIndex -= 2;
    }

    assert('1' <= SAN[endSquareIndex    ] and SAN[endSquareIndex    ] <= '8');
    assert('a' <= SAN[endSquareIndex - 1] and SAN[endSquareIndex - 1] <= 'h');
    int endRank = SAN[endSquareIndex] - '1';
    int endFile = SAN[endSquareIndex - 1] - 'a';

    int endSquare = 8 * endFile + endRank;

    if (promotingPiece != KING) {
        assert(piece == PAWN);
        assert(endRank == (isItWhiteToMove ? 7 : 0));
    }

    int startSquare = 64;
    bool isCapture = SAN.find('x') != std::string::npos;

    if (piece == KING) {
        startSquare = isItWhiteToMove ? whiteKingPosition : blackKingPosition;
    }
    else if (piece == PAWN) {
        unsigned long startSquareMask;
        bool isEnPassant = false;
        bool isDoublePawnPush = false;
        if (isCapture) {
            startSquareMask = isItWhiteToMove ? getMagicBlackAttackedSquares(PAWN_CODE,endSquare,0) : getMagicWhiteAttackedSquares(PAWN_CODE,endSquare,0);
            startSquareMask &= A_FILE << (8 * (SAN[0] - 'a'));
            startSquare = log2ll(startSquareMask);
            assert(1ULL << startSquare == startSquareMask);
            if (isItWhiteToMove)
                isEnPassant = endSquare == whichPawnMovedTwoSquares * 8 + 5;
            else
                isEnPassant = endSquare == whichPawnMovedTwoSquares * 8 + 2;
        }
        else { // it is not a capture
            if (isItWhiteToMove) {
                if ((whitePieceTypes[PAWN_CODE] >> (endSquare - 1) & 1ULL) == 1ULL)
                    startSquare = endSquare - 1;
                else {
                    assert((allWhitePieces >> (endSquare - 1) & 1ULL) == 0ULL);
                    assert((whitePieceTypes[PAWN_CODE] >> (endSquare - 2) & 1ULL) == 1ULL);
                    startSquare = endSquare - 2;
                    isDoublePawnPush = true;
                } // end if it isn't a pawn pushed by 1
            } // end if it is white to move
            else { // it is black to move
                if ((blackPieceTypes[PAWN_CODE] >> (endSquare + 1) & 1ULL) == 1ULL)
                    startSquare = endSquare + 1;
                else {
                    assert((allBlackPieces >> (endSquare + 1) & 1ULL) == 0ULL);
                    assert((blackPieceTypes[PAWN_CODE] >> (endSquare + 2) & 1ULL) == 1ULL);
                    startSquare = endSquare + 2;
                    isDoublePawnPush = true;
                } // end if it isn't a pawn pushed by 1
            } // end if it is black to move
        } // end if it is not a capture
        if (isDoublePawnPush)
            return startSquare << 10 | endSquare << 4 | PAWN_PUSH_TWO_SQUARES_FLAG;
        else if (isEnPassant)
            return startSquare << 10 | endSquare << 4 | EN_PASSANT_FLAG;
        else if (promotingPiece == QUEEN)
            return startSquare << 10 | endSquare << 4 | PROMOTE_TO_QUEEN_FLAG;
        else if (promotingPiece == ROOK)
            return startSquare << 10 | endSquare << 4 | PROMOTE_TO_ROOK_FLAG;
        else if (promotingPiece == BISHOP)
            return startSquare << 10 | endSquare << 4 | PROMOTE_TO_BISHOP_FLAG;
        else if (promotingPiece == KNIGHT)
            return startSquare << 10 | endSquare << 4 | PROMOTE_TO_KNIGHT_FLAG;
    } // end if the piece is a pawn
    else { // the piece is one of QRBN
        vector<move_t> legalMoves;
        getLegalMoves(legalMoves);
        unsigned long possibleStartSquaresMask = 0;
        for (move_t legalMove : legalMoves) {
            if (getMovingPiece(legalMove) == piece and getEndSquare(legalMove) == endSquare)
                possibleStartSquaresMask |= 1ULL << getStartSquare(legalMove);
        }

        // Now we have to deal with if there are other pieces that can go to the same end square
        // So we figure out which one of them is moving
        int rankFileSpecifierIndex = 1;
        while (__builtin_popcountll(possibleStartSquaresMask) != 1) {
            if (rankFileSpecifierIndex >= 3) {
                cout << toString() << endl << toFenNotation() << endl << SAN << endl;
                cout << "rankFileSpecifierIndex is >= 3" << endl;
                exit(1);
            }
            char specifier = SAN[rankFileSpecifierIndex];
            if ('a' <= specifier and specifier <= 'h') {
                int file = specifier - 'a';
                possibleStartSquaresMask &= A_FILE << 8 * file;
            }
            else if ('1' <= specifier and specifier <= '8') {
                int rank = specifier - '1';
                possibleStartSquaresMask &= FIRST_RANK << rank;
            }
            rankFileSpecifierIndex++;
        }

        startSquare = log2ll(possibleStartSquaresMask);
    } // end else (if the piece is one of QRBN)

    assert(startSquare != 64);
    assert(endSquare != 64);
    if (isCapture)
        return getCaptureMove(startSquare, endSquare);
    else
        return startSquare << 10 | endSquare << 4 | NORMAL_MOVE_FLAG;
} // end getMoveFromSAN method

void ChessBoard::makeSANMove (const string& SAN) {
    move_t move = getMoveFromSAN(SAN);

    vector<move_t> legalMoves;
    getLegalMoves(legalMoves);

    if (std::count(legalMoves.begin(), legalMoves.end(),move) == 0) {
        cout << toString() << endl;
        cout << SAN << endl;
        cout << move << endl;
        cout << "This move was illegally attempted" << endl;
        exit(1);
    }

    makemove(move);
}

move_t ChessBoard::getMoveFromPureAlgebraicNotation (const string& pureAlgebraicNotation) const  {
    int startSquare;
    int endSquare;
    move_t flag;
    if (pureAlgebraicNotation.length() == 4) {
        int startFile = pureAlgebraicNotation[0] - 'a';
        int startRank = pureAlgebraicNotation[1] - '1';
        int endFile = pureAlgebraicNotation[2] - 'a';
        int endRank = pureAlgebraicNotation[3] - '1';

        for (int coordinate : {startFile,startRank,endFile,endRank}) {
            assert(0 <= coordinate and coordinate <= 7);
        }

        startSquare = 8 * startFile + startRank;
        endSquare = 8 * endFile + endRank;

        // handle en passant and castling
        const unsigned long allPawns = whitePieceTypes[PAWN_CODE] | blackPieceTypes[PAWN_CODE];
        const bool isAPawnMoving = (allPawns >> startSquare & 1ULL) == 1ULL;

        if (isAPawnMoving and isItWhiteToMove and endSquare == 8 * whichPawnMovedTwoSquares + 5)
            return (startSquare << 10) + (endSquare << 4) + EN_PASSANT_FLAG;
        if (isAPawnMoving and !isItWhiteToMove and endSquare == 8 * whichPawnMovedTwoSquares + 2)
            return (startSquare << 10) + (endSquare << 4) + EN_PASSANT_FLAG;
        else if (isItWhiteToMove and pureAlgebraicNotation == "e1g1" and canWhiteCastleShort)
            return WHITE_SHORT_CASTLE;
        else if (isItWhiteToMove and pureAlgebraicNotation == "e1c1" and canWhiteCastleLong)
            return WHITE_LONG_CASTLE;
        else if (!isItWhiteToMove and pureAlgebraicNotation == "e8g8" and canBlackCastleShort)
            return BLACK_SHORT_CASTLE;
        else if (!isItWhiteToMove and pureAlgebraicNotation == "e8c8" and canBlackCastleLong)
            return BLACK_LONG_CASTLE;
        else {
            move_t baseMove = startSquare << 10 | endSquare << 4 | NORMAL_MOVE_FLAG;
            if (((allWhitePieces | allBlackPieces) >> endSquare & 1ULL) == 1ULL)
                return getCaptureMove(startSquare,endSquare);
            else {
                if (startSquare % 8 == 1 and endSquare % 8 == 3 and getMovingPiece(baseMove) == PAWN)
                    return startSquare << 10 | endSquare << 4 | PAWN_PUSH_TWO_SQUARES_FLAG;
                else if (startSquare % 8 == 6 and endSquare % 8 == 4 and getMovingPiece(baseMove) == PAWN)
                    return startSquare << 10 | endSquare << 4 | PAWN_PUSH_TWO_SQUARES_FLAG;
                else
                    return baseMove;
            }
        } // end else (the move is not castling
    } // end if string length is 4
    else if (pureAlgebraicNotation.length() == 5) {
        int startFile = pureAlgebraicNotation[0] - 'a';
        int startRank = pureAlgebraicNotation[1] - '1';
        int endFile = pureAlgebraicNotation[2] - 'a';
        int endRank = pureAlgebraicNotation[3] - '1';

        for (int coordinate : {startFile,startRank,endFile,endRank}) {
            assert(0 <= coordinate and coordinate <= 7);
        }

        startSquare = 8 * startFile + startRank;
        endSquare = 8 * endFile + endRank;

        switch(pureAlgebraicNotation[4]) {
            case 'q': return (startSquare << 10) + (endSquare << 4) + PROMOTE_TO_QUEEN_FLAG;
            case 'r': return (startSquare << 10) + (endSquare << 4) + PROMOTE_TO_ROOK_FLAG;
            case 'b': return (startSquare << 10) + (endSquare << 4) + PROMOTE_TO_BISHOP_FLAG;
            case 'n': return (startSquare << 10) + (endSquare << 4) + PROMOTE_TO_KNIGHT_FLAG;
            default: assert(false);
        }
    }
    else {
        assert(false);
    }
}

bool ChessBoard::makePureAlgebraicNotationMove (const string& pureAlgebraicNotation) {
    move_t move = getMoveFromPureAlgebraicNotation(pureAlgebraicNotation);
    vector<move_t> legalMoves;
    getLegalMoves(legalMoves);

    if (std::count(legalMoves.begin(), legalMoves.end(),move) == 0)
        return false;
    makemove(move);
    return true;
}

string ChessBoard::moveToPureAlgebraicNotation(move_t move) {
    if (move == WHITE_SHORT_CASTLE)
        return "e1g1";
    if (move == WHITE_LONG_CASTLE)
        return "e1c1";
    if (move == BLACK_SHORT_CASTLE)
        return "e8g8";
    if (move == BLACK_LONG_CASTLE)
        return "e8c8";

    string output;
    output += 'a' + getStartSquare(move) / 8;
    output += '1' + getStartSquare(move) % 8;
    output += 'a' + getEndSquare(move) / 8;
    output += '1' + getEndSquare(move) % 8;
    if (isPromotion(move))
        output += "qrbn"[getFlag(move) - PROMOTE_TO_QUEEN_FLAG];
    return output;
}

string ChessBoard::toFenNotation () const {
    string fenNotation;
    int numEmptySquares;
    // Step 1: Pieces
    for (int rank = 7; rank >= 0; rank--) {
        numEmptySquares = 0;
        for (int file = 0; file < 8; file++) {
            int square = 8 * file + rank;
            unsigned long squareMask = 1ULL << square;
            char piece;
            if (whiteKingPosition == square) {
                piece = 'K';
            }
            else if (blackKingPosition == square) {
                piece = 'k';
            }
            else if (squareMask & whitePieceTypes[QUEEN_CODE]) {
                piece = 'Q';
            }
            else if (squareMask & blackPieceTypes[QUEEN_CODE]) {
                piece = 'q';
            }
            else if (squareMask & whitePieceTypes[ROOK_CODE]) {
                piece = 'R';
            }
            else if (squareMask & blackPieceTypes[ROOK_CODE]) {
                piece = 'r';
            }
            else if (squareMask & whitePieceTypes[BISHOP_CODE]) {
                piece = 'B';
            }
            else if (squareMask & blackPieceTypes[BISHOP_CODE]) {
                piece = 'b';
            }
            else if (squareMask & whitePieceTypes[KNIGHT_CODE]) {
                piece = 'N';
            }
            else if (squareMask & blackPieceTypes[KNIGHT_CODE]) {
                piece = 'n';
            }
            else if (squareMask & whitePieceTypes[PAWN_CODE]) {
                piece = 'P';
            }
            else if (squareMask & blackPieceTypes[PAWN_CODE]) {
                piece = 'p';
            }
            else {
                piece = ' ';
            }
            // We are done finding the piece
            if (piece == ' ') {
                numEmptySquares++;
                if (file == 7)
                    fenNotation += string(1,'0' + numEmptySquares);
            }
            else { // there is an actual piece there
                if (numEmptySquares != 0) {
                    fenNotation += string(1,'0' + numEmptySquares);
                    numEmptySquares = 0;
                }
                fenNotation += string(1,piece);
            }
        } // end for loop over file
        fenNotation += "/";
    } // end for loop over rank

    // Step 2: Whose turn it is to move
    if (isItWhiteToMove) {
        fenNotation += " w ";
    }
    else {
        fenNotation += " b ";
    }

    // Step 3: Castling rights
    if (canWhiteCastleShort)
        fenNotation += "K";
    if (canWhiteCastleLong)
        fenNotation += "Q";
    if (canBlackCastleShort)
        fenNotation += "k";
    if (canBlackCastleLong)
        fenNotation += "q";
    if (!(canWhiteCastleShort or canWhiteCastleLong or canBlackCastleShort or canBlackCastleLong))
        fenNotation += "-";

    fenNotation += " ";

    // Step 4: En passant rights
    if (whichPawnMovedTwoSquares < 8) {
        fenNotation += string(1,'a' + whichPawnMovedTwoSquares);
        if (isItWhiteToMove)
            fenNotation += "6";
        else
            fenNotation += "3";
    }
    else {
        fenNotation += "-";
    }

    fenNotation += " ";

    // Step 5: Halfmove clock

    // TODO: Actually put in the halfmove clock
    int halfmoveClock = 0;
    fenNotation += to_string(halfmoveClock);

    // Step 6: Fullmove clock. We don't have the information to accurately do it, so we will output 0.
    fenNotation += " 1";

    // And now we return the result
    return fenNotation;
}