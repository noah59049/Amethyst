#include "MoveList.h"
void MoveList::push_back(move_t move) {
    assert(size < 218);
    moveList[size] = move;
    size++;
}