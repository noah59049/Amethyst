#pragma once
#include "../movegen/ChessBoard.h"
#include "RepetitionTable.h"
#include "TranspositionTable.h"
#include "TwoKillerMoves.h"
#include "QuietHistory.h"
#include "../UCI.h"
namespace search {
    constexpr const static int KILLER_MAX_COUSIN_LEVEL = 2;
    constexpr const static int IID_DEPTH_DECREASE = 1;
    constexpr const static int NMP_REDUCTION = 3;
    constexpr const static eval_t ZERO_WINDOW_RADIUS = 1.0F;
    constexpr const static unsigned int QUIETS_TO_NOT_REDUCE = 8;
    constexpr const static int MIN_LMR_DEPTH = 3;
    constexpr const static eval_t ASPIRATION_WINDOW_RADIUS = 75;
    constexpr const static eval_t FULL_WINDOW_VALUE = 32000;
    constexpr const static eval_t RFP_MARGIN = 150;
    constexpr const static int MAX_RFP_DEPTH = 4;

    class SearchCancelledException : std::exception {

    };

    struct NegamaxData {
        const bool* isCancelled;
        RepetitionTable repetitionTable;
        TranspositionTable transpositionTable = TranspositionTable(uci::HASH_MB * 1000000 / int(sizeof(TTValue)));
        std::vector<TwoKillerMoves> killerMoves;
        QuietHistory whiteHHB;
        QuietHistory blackHHB;

        NegamaxData(bool* isCancelled, const RepetitionTable& repetitionTable, int depth) {
            this->isCancelled = isCancelled;
            this->repetitionTable = repetitionTable;
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

    eval_t getNegaQuiescenceEval(ChessBoard &board, eval_t alpha, eval_t beta);

    eval_t getNegamaxEval(ChessBoard &board, int depth, eval_t alpha, eval_t beta, NegamaxData& data);

    void getNegamaxBestMoveAndEval(ChessBoard &board, int depth, NegamaxData& data, eval_t aspirationWindowCenter,
                                   move_t &bestMove, eval_t &eval);

    void timeSearchFromFEN (const std::string& fenNotation, int maxDepth);

    void timeSearchFromTraxler ();
} // end namespace search