#include "Conthist.h"
void Conthist::clear() {
    memset(cutoffCounts,0,sizeof(cutoffCounts));
}
void Conthist::recordKillerMove(MoveList &moves, const ChessBoard &previousBoard, move_t previousMove,
                                const ChessBoard &thisBoard, move_t thisMove, int weight) {
    const uint16_t prevIndex = previousBoard.getConthistIndex(previousMove);
    const unsigned int prevIsCapture = isCapture(previousMove);

    for (const move_t move : moves) {
        uint16_t moveIndex = thisBoard.getConthistIndex(move);
        if (move == thisMove) {
            cutoffCounts[prevIsCapture][prevIndex][moveIndex] += weight - weight * cutoffCounts[prevIsCapture][prevIndex][moveIndex] / 512;
            break;
        }
        else if (!isCapture(move)) {
            cutoffCounts[prevIsCapture][prevIndex][moveIndex] -= weight + weight * cutoffCounts[prevIsCapture][prevIndex][moveIndex] / 512;
        }
    }
}

int16_t Conthist::getCutoffCount(const ChessBoard& previousBoard, move_t previousMove, const ChessBoard& thisBoard, move_t thisMove) const {
    return cutoffCounts[isCapture(previousMove)][previousBoard.getConthistIndex(previousMove)][thisBoard.getConthistIndex(thisMove)];
}