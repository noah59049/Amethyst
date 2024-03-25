#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "Flags.h"
#include "../Typedefs.h"

class ChessBoard {
private:
    // Part 1: Fields
    uint8_t whiteKingPosition;
    uint8_t blackKingPosition;

    bitboard_t whitePieceTypes[5] = {1ULL << 24, (1ULL << 0) + (1ULL << 56), (1ULL << 16) + (1ULL << 40),
                                        (1ULL << 8) + (1ULL << 48), 0x0202020202020202ULL};
    // As initialized in default constructor to start up a new game.
    bitboard_t allWhitePieces;

    bitboard_t blackPieceTypes[5] = {1ULL << 31, (1ULL << 7) + (1ULL << 63), (1ULL << 23) + (1ULL << 47),
                                        (1ULL << 15) + (1ULL << 55), 0x4040404040404040ULL};
    // As initialized in default constructor to start up a new game.
    bitboard_t allBlackPieces;

    /**
     * 0 if the a-pawn moved two squares, 4 if the e-pawn moved two squares, 7 if the h-pawn moved two squares, 255 if no pawn moved two squares.
     * This is used for en passant captures.
     */
    uint8_t whichPawnMovedTwoSquares;
    bool isItWhiteToMove;
    bool canWhiteCastleShort;
    bool canWhiteCastleLong;
    bool canBlackCastleShort;
    bool canBlackCastleLong;
    bool drawByInsufficientMaterial;
    zobrist_t zobristCode;
    uint8_t pieceGivingCheck;
    constexpr const static uint8_t DOUBLE_CHECK_CODE = 128;
    constexpr const static uint8_t NOT_IN_CHECK_CODE = 255;
    bool drawByStalemate;
    bool whiteWonByCheckmate;
    bool blackWonByCheckmate;

    // Part 2: FEN constructor
    explicit ChessBoard(const std::string& fenNotation);

    // Part 3: Updating fields
    void updatePieceGivingCheck();
    void updateMates();
    void manuallyInitializeZobristCode();
    void updateDrawByInsufficientMaterial();

    // Part 4: Move generation
    move_t getCaptureMove(int startSquare, int endSquare) const;
    bitboard_t calculateWhiteAttackedSquares() const;
    bitboard_t calculateBlackAttackedSquares() const;
    uint8_t calculatePieceGivingCheck() const;
    void addCastling (std::vector<move_t>& legalMoves, bitboard_t enemyAttackedSquares) const;
    void addEnPassant (std::vector<move_t>& legalMoves, bitboard_t effectiveEnemyBishops, bitboard_t effectiveEnemyRooks) const;
    void addLegalKingMoves (std::vector<move_t>& legalMoves, bitboard_t kingLegalEndSquares) const;

    // Part 5: SEE
    int getCaptureSEE (int capturingPieceType, move_t captureMove) const;
    int getQuietSEE (int movePieceType, move_t quietMove) const;
    int getQuietSEE (move_t move) const;

    // Part 6: Eval
    float getStaticEval () const;

    // Part 7: Algebraic notation
    void decomposeMove(move_t move, Piece& piece, bool& isCapture, int& startSquare, int& endSquare, move_t& flag) const;
    Piece getMovingPiece(move_t move) const;
    [[nodiscard]] move_t getMoveFromSAN (const std::string& SAN) const;

public:
    // Part 1: Constructor
    ChessBoard();
    // Part 2: FEN notation + ASCII art board printout
    static ChessBoard boardFromFENNotation(const std::string& fenNotation) {
        return ChessBoard(fenNotation);
    }
    std::string toString() const;
    std::string toFenNotation () const;

    // Part 2: Move generation
    void makemove(move_t move);
    void getLegalMoves(std::vector<move_t>& legalMoves) const;
    void getLegalCapturesOnly (std::vector<move_t>& captures) const;
    void getNonnegativeSEECapturesOnly (std::vector<move_t>& captures) const;
    void getLegalMoves (std::vector<move_t>& winningEqualCaptures, std::vector<move_t>& losingCaptures, std::vector<move_t>& nonCaptures) const;
    void getLegalMoves (std::vector<move_t>& winningEqualCaptures, std::vector<move_t>& losingCaptures, std::vector<move_t>& zeroSEEQuiets, std::vector<move_t>& negativeSEEQuiets) const;
    bool areThereLegalMoves() const;

    // Part 3: Perft
    int perft (int depth) const;
    int capturePerft (int depth) const;

    // Part 4: Eval
    float getNegaStaticEval () const;

    // Part 5: Null moves
    bool canMakeNullMove () const;
    void makeNullMove ();

    // Part 6: Algebraic notation
    std::string moveToSAN (move_t move) const;
    void makeSANMove (const std::string& SAN);
    move_t getMoveFromPureAlgebraicNotation (const std::string& pureAlgebraicNotation) const;
    bool makePureAlgebraicNotationMove (const std::string& pureAlgebraicNotation);
    static std::string moveToPureAlgebraicNotation(move_t move);

    // Part 7: Getters
    bool isDrawByInsufficientMaterial() const {
        return drawByInsufficientMaterial;
    }
    bool isDrawByStalemate() const {
        //getLegalMoves();
        return drawByStalemate;
    }
    bool isWhiteWonByCheckmate() const {
        //getLegalMoves();
        return whiteWonByCheckmate;
    }
    bool isBlackWonByCheckmate() const {
        //getLegalMoves();
        return blackWonByCheckmate;
    }
    bool hasGameEnded() const {
        return isDrawByInsufficientMaterial() or isDrawByStalemate() or isWhiteWonByCheckmate() or isBlackWonByCheckmate();
    }
    zobrist_t getZobristCode() const {
        return zobristCode;
    }
    bool getIsItWhiteToMove () const {
        return isItWhiteToMove;
    }
    bitboard_t getAllBlackPieces () const {
        return allBlackPieces;
    }
    bool isInCheck () const {
        return pieceGivingCheck != NOT_IN_CHECK_CODE;
    }


};