#pragma once
#include "typedefs.h"
#include "flags.h"
#include "movelist.h"

#include <array>
#include <string>

std::string moveToLAN(move_t move);

class ChessBoard {
private:
    constexpr const static uint8_t DOUBLE_CHECK_CODE = 128;
    constexpr const static uint8_t NOT_IN_CHECK_CODE = 255;

    std::array<bitboard_t, 6> pieceTypes{};
    std::array<bitboard_t, 2> colors{};
    zobrist_t zobristCode{};
    zobrist_t pawnKey{};
    uint16_t halfmove;
    uint16_t fullmove;
    uint8_t epCastlingRights;
    side_t stm;
    square_t pieceGivingCheck;

    // Constructs a board from a FEN string
    explicit ChessBoard(const std::string& fen);
public:
    // Static factory method that returns a position initialized to startpos
    static ChessBoard startpos();

    // Static factory method that returns a position initialized to the specific FEN
    static ChessBoard fromFEN(const std::string& fen);

    // Gives the position in FEN notation
    [[nodiscard]] std::string toFEN() const;

    // Makes sure that the bitboards are all correct
    // This means that
    // 1) bitboards for white and black don't overlap
    // 2) bitboards for two different piece types don't overlap
    // 3) union of bitboard for both sides is the same as union of bitboards for all piece types
    // 4) both sides have exactly one king
    // 5) no pawns are on the first or eighth rank
    // 6) incrementally updated zobrist code is equal to calculated zobrist code
    // If they are all correct, returns true
    // If one of them is false, prints information about which one is false, bitboards, and returns false
    bool areBitboardsCorrect() const;

    // Prints out information about the bitboards of the position
    // This is useful for debugging purposes if the bitboards are not correct
    // But it is not used in the main functions of the engine
    void printAllBitboards() const;

    // Prints out all the legal moves in this position
    // This is useful for debugging purposes
    void printLegalMoves() const;

    // Makes the given move
    // If the move is not a valid move in this position, the behavior is undefined.
    void makemove(move_t move);

    // Makes a null move on the board
    // Does not update halfmove or fullmove
    void makeNullMove();

    // Translates the given move from long algebraic notation (aka uci notation for moves) into a move_t
    [[nodiscard]] move_t parseLANMove(const std::string& move) const;

    // Makes the long-algebraic notation move on the given board
    inline void makeLANMove(const std::string& move) {
        makemove(parseLANMove(move));
    }

    // Returns true if the side to move has a legal move that can take the king
    // This is useful for determining if a pseudolegal move is legal.
    [[nodiscard]] bool canTheKingBeTaken() const;

    // Determines if the move is pseudolegal
    // This means, is there a piece on the start square that could move to the end square
    // That doesn't capture a friendly piece
    // And the flag must be right as well
    // And weird edge cases for en passant and castling
    // We use this method when fetching moves from the TT
    // To prevent crashes in the case of zobrist collisions
    // This assumes that the move is either 0 or was pseudolegal in some position
    // The behavior is undefined if the move is nonzero and isn't pseudolegal in any position
    [[nodiscard]] bool isPseudolegal(move_t move) const;

    // Determines if the move is legal
    // Note that this is only determining "does this move not expose my king to check (and the legality check for castling through check)"
    // And this function assumes the move is pseodolegal
    // The behavior is undefined if the move isn't pseudolegal
    [[nodiscard]] bool isLegal(move_t move) const;

    // Returns true if the move has SEE >= 0, false otherwise
    // The behavior is undefined if the move isn't pseudolegal
    [[nodiscard]] bool isGoodSEE(move_t move) const;

    // Sets the pieceGivingCheck field to whichever square is the piece giving check (if any)
    void updatePieceGivingCheck();

    // Returns the type of the piece on the square (e.g. pawn knight bishop etc.)
    // Will throw an error and exit if there is no piece on that square
    [[nodiscard]] piece_t getPieceAt(square_t square) const;

    // Gets a list of all the pseudolegal moves in the position
    [[nodiscard]] MoveList getPseudoLegalMoves() const;

    // Gets a list of either tactical or quiet moves
    // We use this for staged movegen
    void getMoves(MoveList& moves, BasicMovegenStage stage) const;

    // Gets the zobrist code, calculated by adding up all the pieces and rights and stm from scratch.
    // This function is slow.
    [[nodiscard]] zobrist_t calcZobristCode() const;

    // Gets the incrementally updated zobrist code.
    // This function is fast.
    [[nodiscard]] zobrist_t getZobristCode() const;

    // Gets the pawn key, which is like a zobrist code, but it only tracks pawns.
    // This function is slow.
    [[nodiscard]] zobrist_t calcPawnKey() const;

    // Gets the incrementally updated pawn key
    // This function is fast.
    [[nodiscard]] zobrist_t getPawnKey() const;

    // Returns true if we can attempt null move pruning
    [[nodiscard]] bool canTryNMP() const;

    // Determines if the position is in check.
    [[nodiscard]] inline bool isInCheck() const {
        return pieceGivingCheck != NOT_IN_CHECK_CODE;
    }

    // Gets the side to move.
    [[nodiscard]] inline side_t getSTM() const {
        return stm;
    }

    // Gets the halfmove, or how many moves have been made since a pawn move or a capture.
    [[nodiscard]] inline auto getHalfmove() const {
        return halfmove;
    }

    [[nodiscard]] inline bitboard_t getPieceBB(piece_t piece) const {
        return pieceTypes[piece];
    }

    [[nodiscard]] inline bitboard_t getSideBB(side_t side) const {
        return colors[side];
    }
};