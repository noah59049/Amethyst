#pragma once
#include <array>
#include "../Typedefs.h"
struct MoveList {
    std::array<move_t,218> moveList;
    unsigned int size = 0;
    MoveList()=default;
    inline void push_back(move_t move) {
        assert(size < 218);
        moveList[size] = move;
        size++;
    }

    [[nodiscard]] inline move_t at (const unsigned int index) const {
        return moveList[index];
    }

    inline void trimToSize(const unsigned int newSize) {
        if (size > newSize)
            size = newSize;
    }

    inline std::array<move_t,218>::iterator begin() {
        return moveList.begin();
    }
    inline std::array<move_t,218>::iterator end() {
        return moveList.begin() + size;
    }
};