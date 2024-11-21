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
    uint16_t halfmove;
    uint16_t fullmove;
    uint8_t epCastlingRights;
    side_t stm;
    square_t pieceGivingCheck;


    explicit ChessBoard(const std::string& fen);
public:
    static ChessBoard startpos();
    static ChessBoard fromFEN(const std::string& fen);
    [[nodiscard]] std::string toFEN() const;
    [[nodiscard]] bool areBitboardsCorrect() const;
    void printAllBitboards() const;
    void makemove(move_t move);
    [[nodiscard]] move_t parseLANMove(const std::string& move) const;
    inline void makeLANMove(const std::string& move) {
        makemove(parseLANMove(move));
    }
    [[nodiscard]] bool canTheKingBeTaken() const;
    [[nodiscard]] bool isLegal(move_t move) const;
    void updatePieceGivingCheck();
    [[nodiscard]] piece_t getPieceAt(square_t square) const;
    [[nodiscard]] MoveList getPseudoLegalMoves() const;
    [[nodiscard]] zobrist_t calcZobristCode() const;
    [[nodiscard]] zobrist_t getZobristCode() const;

    [[nodiscard]] eval_t getEval() const;

    [[nodiscard]] inline bool isInCheck() const {
        return pieceGivingCheck != NOT_IN_CHECK_CODE;
    }
};