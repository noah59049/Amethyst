#pragma once
#include <vector>
#include "movegen/ChessBoard.h"
class ChessGame {
private:
    ChessBoard currentPosition;
    std::vector<ChessBoard> positions;
public:
    ChessGame() =default;
    explicit ChessGame(const std::vector<ChessBoard>& positions) {
        this->positions = positions;
        currentPosition = positions.at(positions.size() - 1);
    }
    explicit ChessGame(const ChessBoard& position) {
        this->currentPosition = position;
        this->positions = {position};
    }
    void makemove (move_t move) {
        currentPosition.makemove(move);
        positions.push_back(currentPosition);
    }
    std::vector<ChessBoard> getPositions() const {
        return positions;
    }

    ChessBoard getCurrentPosition() const {
        return currentPosition;
    }

    bool hasGameEnded() const {
        return currentPosition.hasGameEnded();
    }
};