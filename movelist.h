#pragma once
#include <array>
#include "typedefs.h"
struct MoveList {
    std::array<move_t,256> moveList;
    unsigned int size = 0;
    MoveList()=default;
    inline void push_back(move_t move) {
        assert(size < 256);
        moveList[size] = move;
        size++;
    }

    inline move_t pop_back() {
        assert(size > 0);
        size--;
        return moveList[size];
    }

    inline void trimToSize(const unsigned int newSize) {
        if (size > newSize)
            size = newSize;
    }

    [[nodiscard]] inline move_t at (const unsigned int index) const {
        return moveList[index];
    }

    inline std::array<move_t,256>::iterator begin() {
        return moveList.begin();
    }
    inline std::array<move_t,256>::iterator end() {
        return moveList.begin() + size;
    }
};