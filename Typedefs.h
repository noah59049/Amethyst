#pragma once
#include <cstdint>
using bitboard_t = uint64_t;
using move_t = uint16_t;
using zobrist_t = uint64_t;
using see_mask = int32_t;
using packed_eval_t = uint64_t;
using eval_t = int;
using ttbound_t = int16_t;
using colored_piece_t = uint16_t;
using depth_t = int8_t;