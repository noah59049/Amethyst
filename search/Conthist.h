#pragma once
#include "../Typedefs.h"
#include "../movegen/MoveList.h"
#include "../movegen/ChessBoard.h"
class Conthist {
private:
    // [isCapture][previousMove][cutoffMove]
    int16_t cutoffCounts[2][64 * 12][64 * 12];
public:
    void clear ();
    void recordKillerMove(MoveList& moves, const ChessBoard& previousBoard, move_t previousMove, const ChessBoard& thisBoard, move_t thisMove, int weight);
    [[nodiscard]] int16_t getCutoffCount(const ChessBoard& previousBoard, move_t previousMove, const ChessBoard& thisBoard, move_t thisMove) const;
};