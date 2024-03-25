#pragma once
#include "../Typedefs.h"
namespace bitmasks {
    constexpr const static bitboard_t INNER_RANKS = 0x7e7e7e7e7e7e7e7eULL;
    constexpr const static bitboard_t INNER_FILES = 0x00ffffffffffff00ULL;
    constexpr const static bitboard_t INNER_36 = 0x007e7e7e7e7e7e00ULL;

    constexpr const static bitboard_t A_FILE = 255ULL;
    constexpr const static bitboard_t H_FILE = 0xff00000000000000ULL;
    constexpr const static bitboard_t FIRST_RANK = 0x0101010101010101ULL;
    constexpr const static bitboard_t SECOND_RANK = 0x0202020202020202ULL;
    constexpr const static bitboard_t SEVENTH_RANK = 0x4040404040404040ULL;
    constexpr const static bitboard_t EIGHTH_RANK = 0x8080808080808080ULL;

    constexpr const static bitboard_t NOT_A_FILE = ~A_FILE;
    constexpr const static bitboard_t NOT_H_FILE = ~H_FILE;

    constexpr const static bitboard_t E1_F1_G1 = 0x0001010100000000ULL;
    constexpr const static bitboard_t E1_D1_C1 = 0x0000000101010000ULL;
    constexpr const static bitboard_t E8_F8_G8 = 0x0080808000000000ULL;
    constexpr const static bitboard_t E8_D8_C8 = 0x0000008080800000ULL;

    constexpr const static bitboard_t E1_H1 = 0x0100000100000000ULL;
    constexpr const static bitboard_t E1_A1 = 0x0000000100000001ULL;
    constexpr const static bitboard_t E1_THROUGH_H1 = 0x0101010100000000ULL;
    constexpr const static bitboard_t E1_THROUGH_A1 = 0x0000000101010101ULL;
    constexpr const static bitboard_t E8_H8 = 0x8000008000000000ULL;
    constexpr const static bitboard_t E8_A8 = 0x0000008000000080ULL;
    constexpr const static bitboard_t E8_THROUGH_H8 = 0x8080808000000000ULL;
    constexpr const static bitboard_t E8_THROUGH_A8 = 0x0000008080808080ULL;

    constexpr const static bitboard_t LIGHT_SQUARES = 0x55AA55AA55AA55AAULL;
    constexpr const static bitboard_t DARK_SQUARES = 0xAA55AA55AA55AA55ULL;

    constexpr const static bitboard_t ENTIRE_BOARD = LIGHT_SQUARES | DARK_SQUARES;
}