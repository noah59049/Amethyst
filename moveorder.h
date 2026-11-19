#pragma once
#include "movelist.h"
#include <cstdint>

uint32_t getMVVLVAScore(move_t move);

void scoreMovesByMVVLVA(MoveList& moves);