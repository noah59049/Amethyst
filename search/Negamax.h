#pragma once
#include "../movegen/ChessBoard.h"
#include "RepetitionTable.h"
#include "TranspositionTable.h"
#include "TwoKillerMoves.h"
#include "QuietHistory.h"
namespace search {
    constexpr const static int KILLER_MAX_COUSIN_LEVEL = 2;
    constexpr const static int IID_DEPTH_DECREASE = 1;
    constexpr const static int NMP_REDUCTION = 3;
    constexpr const static float ZERO_WINDOW_RADIUS = 1.0F;
    constexpr const static unsigned int QUIETS_TO_NOT_REDUCE = 8;
    constexpr const static int MIN_LMR_DEPTH = 3;
    constexpr const static float ASPIRATION_WINDOW_RADIUS = 75;
    constexpr const static float FULL_WINDOW_VALUE = 2000000;

    class SearchCancelledException : std::exception {

    };

    struct NegamaxData {
        const bool* isCancelled;
        RepetitionTable repetitionTable;
        TranspositionTable transpositionTable;
        std::vector<TwoKillerMoves> killerMoves;
        QuietHistory whiteHHB;
        QuietHistory blackHHB;

        NegamaxData(bool* isCancelled, const RepetitionTable& repetitionTable, const TranspositionTable& transpositionTable, const std::vector<TwoKillerMoves>& killerMoves, const QuietHistory& whiteHHB, const QuietHistory& blackHHB) {
            this->isCancelled = isCancelled;
            this->repetitionTable = repetitionTable;
            this->transpositionTable = transpositionTable;
            this->killerMoves = killerMoves;
            this->whiteHHB = whiteHHB;
            this->blackHHB = blackHHB;
        }

        NegamaxData(bool* isCancelled, const RepetitionTable& repetitionTable, int depth) {
            this->isCancelled = isCancelled;
            this->repetitionTable = repetitionTable;
            this->transpositionTable = TranspositionTable();
            this->killerMoves = std::vector<TwoKillerMoves>(depth + 1);
            this->whiteHHB = QuietHistory();
            this->blackHHB = QuietHistory();
        }

        void extendKillersToDepth(const int depth) {
            while (killerMoves.size() <= depth) {
                killerMoves.emplace_back();
            }
        }
    };

    float getNegaQuiescenceEval(const ChessBoard &board, float alpha, float beta);

    float getNegamaxEval(const ChessBoard &board, int depth, float alpha, float beta, NegamaxData& data);

    void getNegamaxBestMoveAndEval(const ChessBoard &board, int depth, NegamaxData& data, float aspirationWindowCenter,
                                   move_t &bestMove, float &eval);

    void timeSearchFromFEN (const std::string& fenNotation, int maxDepth);

    void timeSearchFromTraxler ();
} // end namespace search