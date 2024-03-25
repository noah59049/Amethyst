#pragma once
#include "../Typedefs.h"
class TwoKillerMoves {
private:
    constexpr const static move_t NULL_KILLER_MOVE = 2080; //a2a2
    move_t firstKillerMove;
    int8_t firstKillerMoveCount;
    move_t secondKillerMove;
    int8_t secondKillerMoveCount;
public:
    TwoKillerMoves();

    void clear();

    void recordKillerMove(move_t move);

    [[nodiscard]] inline move_t getFirstKillerMove() const {
        return firstKillerMove;
    }

    [[nodiscard]] inline move_t getSecondKillerMove() const {
        return secondKillerMove;
    }
}; // end class