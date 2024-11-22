#pragma once

#include "typedefs.h"

namespace hce {
    constexpr phase_t PHASE_PIECE_VALUES[6] = {0,1,1,2,4,0};
    constexpr phase_t MAX_PHASE = 24;

    constexpr packed_eval_t S(int16_t mg, int16_t eg) {
        return (uint32_t(uint16_t(mg)) << 16) + eg;
    }

    constexpr packed_eval_t material[6] = {S(102, 196), S(394, 511), S(432, 538), S(499, 919), S(1103, 1628), S(0, 0)};

    inline eval_t evalFromPacked(packed_eval_t packed, phase_t phase) {
        int32_t mg = int32_t(int16_t(uint16_t((packed + (1U << 15)) >> 16)));
        int32_t eg = int32_t(int16_t(uint16_t(packed)));
        if (phase >= MAX_PHASE)
            return mg;
        else
            return (mg * phase + eg * (phase - MAX_PHASE)) / MAX_PHASE;
    }
}