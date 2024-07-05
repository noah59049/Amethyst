#pragma once
#include "../movegen/ChessBoard.h"
#include "RepetitionTable.h"
#include "TranspositionTable.h"
#include "TwoKillerMoves.h"
#include "QuietHistory.h"
#include "../UCI.h"
namespace search {
    constexpr const static int KILLER_MAX_COUSIN_LEVEL = 2;
    constexpr const static int IID_DEPTH_DECREASE = 5;
    constexpr const static int NMP_REDUCTION = 3;
    constexpr const static int MIN_LMR_DEPTH = 3;
    constexpr const static unsigned int QUIETS_TO_NOT_REDUCE = 8;
    constexpr const static int SECOND_LMR_DEPTH = 7;
    constexpr const static unsigned int SECOND_QUIETS_TO_NOT_REDUCE = 5;
    constexpr const static int MIN_FR_DEPTH = 3;
    constexpr const static int MIN_SEE_REDUCTION_DEPTH = 5;
    constexpr const static int FR_MARGIN = 256;
    constexpr const static eval_t ASPIRATION_WINDOW_RADIUS = 75;
    constexpr const static eval_t RFP_MARGIN = 150;
    constexpr const static int MAX_RFP_DEPTH = 4;
    constexpr const static unsigned int MAX_LMP_DEPTH = 3;
    constexpr const static unsigned int LMP_MOVECOUNT = 7;
    constexpr const static unsigned int MIN_NMP_DEPTH = 3;

    constexpr const static eval_t MAX_EVAL = 32767;
    constexpr const static eval_t MIN_EVAL = -32768;
    constexpr const static eval_t FULL_WINDOW_VALUE = 32000;
    constexpr const static eval_t FAIL_SOFT_MIN_SCORE = -31900;

    class SearchCancelledException : std::exception {

    };

    static TranspositionTable GLOBAL_TT = TranspositionTable(uci::HASH_MB * 1000000 / int(sizeof(TTValue)));

    struct NegamaxData {
        const bool* isCancelled;
        RepetitionTable repetitionTable;
        // TranspositionTable transpositionTable = TranspositionTable(uci::HASH_MB * 1000000 / int(sizeof(TTValue)));
        std::vector<TwoKillerMoves> killerMoves;
        QuietHistory whiteQuietHistory;
        QuietHistory blackQuietHistory;

        NegamaxData(bool* isCancelled, const RepetitionTable& repetitionTable, int depth) {
            this->isCancelled = isCancelled;
            this->repetitionTable = repetitionTable;
            this->killerMoves = std::vector<TwoKillerMoves>(depth + 1);
            this->whiteQuietHistory = QuietHistory();
            this->blackQuietHistory = QuietHistory();
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