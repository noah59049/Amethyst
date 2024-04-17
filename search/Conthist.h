#pragma once
#include "../Typedefs.h"
#include "../movegen/MoveList.h"
#include "../movegen/ChessBoard.h"
class Conthist {
private:
    // [isCapture][previousMove][cutoffMove]
    int16_t cutoffCounts[64 * 12 * 2][64 * 12];
public:
    Conthist() {
        clear();
    }
    void clear ();
    void recordKillerMove(MoveList& moves, uint16_t prevConthistIndex, const ChessBoard& thisBoard, move_t thisMove, int weight);
    [[nodiscard]] int16_t getCutoffCount(uint16_t prevConthistIndex, const ChessBoard& thisBoard, move_t thisMove) const;
};