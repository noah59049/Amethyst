#include "Conthist.h"
void Conthist::clear() {
    memset(cutoffCounts,0,sizeof(cutoffCounts));
}
void Conthist::recordKillerMove(MoveList &moves, const uint16_t prevIndex,
                                const ChessBoard &thisBoard, move_t thisMove, int weight) {

    for (const move_t move : moves) {
        uint16_t moveIndex = thisBoard.getConthistIndex(move);
        if (move == thisMove) {
            cutoffCounts[prevIndex][moveIndex] += weight - weight * cutoffCounts[prevIndex][moveIndex] / 512;
            break;
        }
        else if (!isCapture(move)) {
            cutoffCounts[prevIndex][moveIndex] -= weight + weight * cutoffCounts[prevIndex][moveIndex] / 512;
        }
    }
}

int16_t Conthist::getCutoffCount(const uint16_t prevIndex, const ChessBoard& thisBoard, move_t thisMove) const {
    return cutoffCounts[prevIndex][thisBoard.getConthistIndex(thisMove)];
}