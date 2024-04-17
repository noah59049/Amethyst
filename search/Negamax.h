#pragma once
#include "../movegen/ChessBoard.h"
#include "RepetitionTable.h"
#include "TranspositionTable.h"
#include "TwoKillerMoves.h"
#include "QuietHistory.h"
#include "Conthist.h"
#include "../UCI.h"
namespace search {
    constexpr const static int KILLER_MAX_COUSIN_LEVEL = 2;
    constexpr const static int IID_DEPTH_DECREASE = 1;
    constexpr const static int NMP_REDUCTION = 3;
    constexpr const static unsigned int QUIETS_TO_NOT_REDUCE = 8;
    constexpr const static int MIN_LMR_DEPTH = 3;
    constexpr const static eval_t ASPIRATION_WINDOW_RADIUS = 75;
    constexpr const static eval_t FULL_WINDOW_VALUE = 32000;
    constexpr const static eval_t RFP_MARGIN = 150;
    constexpr const static int MAX_RFP_DEPTH = 4;
    constexpr const static int MAX_LMP_DEPTH = 3;
    constexpr const static unsigned int LMP_MOVECOUNT = 6;

    constexpr const static eval_t MAX_EVAL = 32767;
    constexpr const static eval_t MIN_EVAL = -32768;

    class SearchCancelledException : std::exception {

    };

    static RepetitionTable repetitionTable;
    static TranspositionTable transpositionTable = TranspositionTable(uci::HASH_MB * 1000000 / int(sizeof(TTValue)));
    static std::vector<TwoKillerMoves> killerMoves = std::vector<TwoKillerMoves>(100);
    static QuietHistory whiteQuietHistory;
    static QuietHistory blackQuietHistory;
    static Conthist conthist;
    static std::vector<uint16_t> conthistStack;

    inline void setupNewRoot() {
        conthistStack.clear();
        conthistStack.reserve(100);
        transpositionTable = TranspositionTable(uci::HASH_MB * 1000000 / int(sizeof(TTValue)));
        repetitionTable = RepetitionTable();
        whiteQuietHistory.clear();
        blackQuietHistory.clear();
    }

    inline void setupNewGame () {
//        setupNewRoot();
//        transpositionTable = TranspositionTable(uci::HASH_MB * 1000000 / int(sizeof(TTValue)));
//        repetitionTable = RepetitionTable();
//        whiteQuietHistory.clear();
//        blackQuietHistory.clear();
        //conthist.clear();
    }

    eval_t getNegaQuiescenceEval(ChessBoard &board, eval_t alpha, eval_t beta);

    eval_t getNegamaxEval(ChessBoard &board, int depth, eval_t alpha, eval_t beta, bool* isCancelled);

    void getNegamaxBestMoveAndEval(ChessBoard &board, int depth, bool* isCancelled, eval_t aspirationWindowCenter,
                                   move_t &bestMove, eval_t &eval);

//    void timeSearchFromFEN (const std::string& fenNotation, int maxDepth);
//
//    void timeSearchFromTraxler ();
} // end namespace search