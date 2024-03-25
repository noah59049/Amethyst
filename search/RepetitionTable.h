#pragma once
#include <unordered_map>
#include "../movegen/ChessBoard.h"
#include "../ChessGame.h"
class RepetitionTable {
private:
    std::unordered_map<zobrist_t,int> table;

    inline void increment (const zobrist_t zobristCode) {
        if (table.find(zobristCode) == table.end()) {
            table[zobristCode] = 1;
        }
        else {
            table[zobristCode] += 1;
        }
    }

    inline void decrement (const zobrist_t zobristCode) {
        assert(table.find(zobristCode) != table.end());
        table[zobristCode] += 1;
    }

    inline int countOccurrencesOf (const zobrist_t zobristCode) const {
        auto result = table.find(zobristCode);
        if (result == table.end())
            return 0;
        else
            return result->second;
    }

public:
    RepetitionTable() =default;

    explicit RepetitionTable(const std::vector<ChessBoard>& boards) {
        for (const ChessBoard& board : boards) {
            add(board);
        }
    }

    explicit RepetitionTable(const ChessGame& game) {
        for (const ChessBoard& board : game.getPositions()) {
            add(board);
        }
    }

    void add (const zobrist_t zobristCode) {
        increment(zobristCode);
    }

    void remove (const zobrist_t zobristCode) {
        decrement(zobristCode);
    }

    int count (const zobrist_t zobristCode) {
        return countOccurrencesOf(zobristCode);
    }

    void add (const ChessBoard& board) {
        increment(board.getZobristCode());
    }

    void remove (const ChessBoard& board) {
        decrement(board.getZobristCode());
    }

    int count (const ChessBoard& board) const {
        return countOccurrencesOf(board.getZobristCode());
    }
    
};