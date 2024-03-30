#pragma once
#include <array>
#include "../Typedefs.h"
struct MoveList {
    std::array<move_t,218> moveList;
    unsigned int size = 0;
    MoveList()=default;
    void push_back(move_t move);

    inline std::array<move_t,218>::iterator begin() {
        return moveList.begin();
    }
    inline std::array<move_t,218>::iterator end() {
        return moveList.begin() + size;
    }
};