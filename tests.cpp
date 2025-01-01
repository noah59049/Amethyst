#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <regex>
#include <random>
#include <unordered_map>
#include <utility>
#include <functional>
#include <ios>

#include "flags.h"
#include "chessboard.h"
#include "perft.h"
#include "search.h"
#include "moveorder.h"
#include "tt.h"
#include "movegenerator.h"

// I don't think this is really necessary
// But why not leave it in
void printMoveCharacteristics(move_t move) {
    using namespace std;
    cout << "start square is " << mvs::getFrom(move) << endl;
    cout << "end square is " << mvs::getTo(move) << endl;
    cout << "flag is " << mvs::getFlag(move) << endl;
    cout << "moving piece is " << uint16_t(mvs::getPiece(move)) << endl;
    cout << "captured piece is " << uint16_t(mvs::getCapturedPiece(move)) << endl;

    cout << boolalpha;

    cout << "is it capture: " << mvs::isCapture(move) << endl;
    cout << "is it promotion: " << mvs::isPromotion(move) << endl;
    cout << "is it tactical: " << mvs::isTactical(move) << endl;
    cout << "is it quiet: " << mvs::isQuiet(move) << endl;
}

// Tests if the functions in flags.h return the correct outputs when called on this move
// Returns true if the tests all passed
// Otherwise, returns false
bool flagTest(move_t move,
              move_t expectedFrom,
              move_t expectedTo,
              move_t expectedFlag,
              piece_t expectedPiece,
              piece_t expectedCapturedPiece,
              bool expectedIsCapture,
              bool expectedIsPromotion,
              bool expectedIsTactical,
              bool expectedIsQuiet) {

    // Step 0: Initialize havePassedAllTests
    bool havePassedAllTests = true; // we haven't failed any tests yet...

    // Step 1: Check that all the bools are correct
    bool isCapture = mvs::isCapture(move);
    bool isPromotion = mvs::isPromotion(move);
    bool isTactical = mvs::isTactical(move);
    bool isQuiet = mvs::isQuiet(move);
    if (isCapture != expectedIsCapture) {
        havePassedAllTests = false;
    }
    if (isPromotion != expectedIsPromotion) {
        havePassedAllTests = false;
    }
    if (isTactical != expectedIsTactical) {
        havePassedAllTests = false;
    }
    if (isQuiet != expectedIsQuiet) {
        havePassedAllTests = false;
    }

    // Step 2: Check that from, to, flag, and moving piece are all correct
    move_t from = mvs::getFrom(move);
    move_t to = mvs::getTo(move);
    move_t flag = mvs::getFlag(move);
    piece_t piece = mvs::getPiece(move);
    if (from != expectedFrom) {
        havePassedAllTests = false;
    }
    if (to != expectedTo) {
        havePassedAllTests = false;
    }
    if (flag != expectedFlag) {
        havePassedAllTests = false;
    }
    if (piece != expectedPiece) {
        havePassedAllTests = false;
    }

    // Step 3: Check that captured piece is correct
    // This is slightly harder because if the move is not a capture, the captured piece should be 0
    piece_t capturedPiece = mvs::getCapturedPiece(move);
    if (isCapture) {
        if (expectedCapturedPiece != capturedPiece) {
            havePassedAllTests = false;
        }
    }
    else {
        if (capturedPiece != 0) {
            havePassedAllTests = false;
        }
    }

    return havePassedAllTests;
}

bool underpromoTest() {
    move_t move = 0b1000001100001111000110; // a7 pawn captures queen on b8 and promotes to horsey
    move_t expectedFrom = squares::a7;
    move_t expectedTo = squares::b8;
    move_t expectedFlag = flags::KNIGHT_CAP_PROMO_FLAG;
    piece_t expectedPiece = pcs::PAWN;
    piece_t expectedCapturedPiece = pcs::QUEEN;
    bool expectedIsCapture = true;
    bool expectedIsPromotion = true;
    bool expectedIsTactical = true;
    bool expectedIsQuiet = false;

    return flagTest(move, expectedFrom, expectedTo, expectedFlag, expectedPiece, expectedCapturedPiece, expectedIsCapture, expectedIsPromotion, expectedIsTactical, expectedIsQuiet);
}

bool doublePawnPushTest() {
    move_t move = 0b0000000001011011011001; // d2 pawn pushes to d4    move_t expectedFrom = squares::a7;
    move_t expectedFrom = squares::d2;
    move_t expectedTo = squares::d4;
    move_t expectedFlag = flags::DOUBLE_PAWN_PUSH_FLAG;
    piece_t expectedPiece = pcs::PAWN;
    piece_t expectedCapturedPiece = pcs::PAWN;
    bool expectedIsCapture = false;
    bool expectedIsPromotion = false;
    bool expectedIsTactical = false;
    bool expectedIsQuiet = true;

    return flagTest(move, expectedFrom, expectedTo, expectedFlag, expectedPiece, expectedCapturedPiece, expectedIsCapture, expectedIsPromotion, expectedIsTactical, expectedIsQuiet);
}

bool alienGambitTest() {
    move_t move = 0b0000010100101110110100; // alien gambit
    move_t expectedFrom = squares::g5;
    move_t expectedTo = squares::f7;
    move_t expectedFlag = flags::CAPTURE_FLAG;
    piece_t expectedPiece = pcs::KNIGHT;
    piece_t expectedCapturedPiece = pcs::PAWN;
    bool expectedIsCapture = true;
    bool expectedIsPromotion = false;
    bool expectedIsTactical = true;
    bool expectedIsQuiet = false;

    return flagTest(move, expectedFrom, expectedTo, expectedFlag, expectedPiece, expectedCapturedPiece, expectedIsCapture, expectedIsPromotion, expectedIsTactical, expectedIsQuiet);
}

bool promoTest() {
    move_t move = 0b0000001011111000111001; // h2 pawn promotes to queen on h1
    move_t expectedFrom = squares::h2;
    move_t expectedTo = squares::h1;
    move_t expectedFlag = flags::QUEEN_PROMO_FLAG;
    piece_t expectedPiece = pcs::PAWN;
    piece_t expectedCapturedPiece = pcs::PAWN;
    bool expectedIsCapture = false;
    bool expectedIsPromotion = true;
    bool expectedIsTactical = true;
    bool expectedIsQuiet = false;

    return flagTest(move, expectedFrom, expectedTo, expectedFlag, expectedPiece, expectedCapturedPiece, expectedIsCapture, expectedIsPromotion, expectedIsTactical, expectedIsQuiet);
}

// This function runs tests for moves
// I just handpicked these moves
// And handpicked what the outputs should be
// So it is entirely possible that I made a bug in my tests
bool runAllMovesTests () {
    bool passedAllTests = true;

    if (!underpromoTest()) {
        std::cout << "FAILED underpromo test" << std::endl;
        passedAllTests = false;
    }
    if (!doublePawnPushTest()) {
        std::cout << "FAILED doublePawnPush test" << std::endl;
        passedAllTests = false;
    }
    if (!alienGambitTest()) {
        std::cout << "FAILED alienGambit test" << std::endl;
        passedAllTests = false;
    }
    if (!promoTest()) {
        std::cout << "FAILED promo test" << std::endl;
        passedAllTests = false;
    }
    if (passedAllTests) {
        std::cout << "PASSED all handpicked move tests" << std::endl;
    }

    return passedAllTests;
}

bool testMoveConstructor(move_t from, move_t to, move_t flag, piece_t piece, piece_t capturedPiece,
                         bool isCapture, bool isPromotion, bool isTactical, bool isQuiet) {
    move_t move = mvs::constructMove(from,to,flag,piece,capturedPiece);
    return flagTest(move,from,to,flag,piece,capturedPiece,isCapture,isPromotion,isTactical,isQuiet);
}

bool runAllMoveConstructorTests() {
    bool passedAllTests = true;

    if (!testMoveConstructor(squares::g1,squares::f3,flags::QUIET_FLAG,pcs::KNIGHT,0,false,false,false,true)) {
        std::cout << "FAILED Nf3 test" << std::endl;
        passedAllTests = false;
    }
    if (!testMoveConstructor(squares::c4,squares::d5,flags::CAPTURE_FLAG,pcs::BISHOP,pcs::KNIGHT,true,false,true,false)) {
        std::cout << "FAILED Bxd5 test" << std::endl;
        passedAllTests = false;
    }
    if (!testMoveConstructor(squares::e5,squares::f6,flags::EN_PASSANT_FLAG,pcs::PAWN,pcs::PAWN,true,false,true,false)) {
        std::cout << "FAILED enPassant test" << std::endl;
        passedAllTests = false;
    }

    if (passedAllTests)
        std::cout << "PASSED all move constructor tests" << std::endl;

    return passedAllTests;
}

// Tests if making a ChessBoard from this fen and then converting it back to FEN gives the same result
// Returns true if they are the same
// Returns false and prints out the difference if they are not the same
bool testFenParsing(const std::string& fen) {
    ChessBoard board = ChessBoard::fromFEN(fen);
    std::string outFen = board.toFEN();

    if (fen != outFen) {
        std::cout << "FAILED fen parser test:" << std::endl;
        std::cout << "Input fen is:  " << fen << std::endl;
        std::cout << "Output fen is: " << outFen << std::endl;
        std::cout << std::endl;
        return false;
    }
    return board.areBitboardsCorrect(); // We also want to test that
}

// This function was generated with ChatGPT
std::string trimCarriageReturns(const std::string& str) {
    std::string trimmed = str;
    while (!trimmed.empty() && (trimmed.back() == '\r')) {
        trimmed.pop_back();
    }
    return trimmed;
}

bool fenTestSuite(const std::string& filename) {
    std::ifstream file(filename);
    std::string fen;
    int testsPassed = 0;
    int testsFailed = 0;
    while(file.peek() != EOF) {
        getline(file,fen);
        fen = trimCarriageReturns(fen);
        testFenParsing(fen) ? testsPassed++ : testsFailed++;
    }

    file.close();
    std::cout << "passed " << testsPassed << " and failed " << testsFailed << " fen parser tests for " << filename << std::endl;
    if (testsFailed == 0 and testsPassed > 0)
        std::cout << "PASSED fen parser test for " << filename << std::endl;
    return testsFailed == 0 and testsPassed > 0;
}

void startposPerft(depth_t maxDepth) {
    ChessBoard startpos = ChessBoard::startpos();
    for (depth_t depth = 0; depth <= maxDepth; depth++) {
        perft_t result = perft(startpos, depth);
        std::cout << "startpos perft(" << int(depth) << ") = " << result << std::endl;
    }
}

void fenPerft(const std::string& fen, depth_t maxDepth) {
    ChessBoard board = ChessBoard::fromFEN(fen);
    for (depth_t depth = 0; depth <= maxDepth; depth++) {
        perft_t result = perft(board, depth);
        std::cout << "fen " << fen << " perft(" << int(depth) << ") = " << result << std::endl;
    }
}

void splitperft(const std::string& fen, depth_t depth) {
    std::cout << "splitperft(" << fen << ") at depth=" << int(depth) << ":" << std::endl;
    assert(depth >= 1);
    ChessBoard board = ChessBoard::fromFEN(fen);
    MoveList moves = board.getPseudoLegalMoves();
    for (move_t move : moves) {
        ChessBoard newBoard = board;
        newBoard.makemove(move);
        if (!newBoard.canTheKingBeTaken())
            std::cout << moveToLAN(move) << ":" << perft(newBoard,depth - 1) << std::endl;
    }
}

// ChatGPT generated this code
bool runPerftSuiteLine(const std::string& line, bool verbose) {
    std::vector<int> vector1;  // Stores the values after D
    std::vector<perft_t> vector2;  // Stores the other integers

    // Extract the part after the semicolon
    size_t semicolon_pos = line.find(';');
    if (semicolon_pos != std::string::npos) {
        std::string pairs = line.substr(semicolon_pos + 1);

        // Regex to match D<int> <integer> pairs
        std::regex pair_regex(R"(D(\d+)\s+(\d+))");
        std::smatch match;
        std::string::const_iterator search_start = pairs.cbegin();

        // Loop over all matches
        while (std::regex_search(search_start, pairs.cend(), match, pair_regex)) {
            // Extract the pair of integers
            int first_integer = std::stoi(match[1].str());
            int second_integer = std::stoi(match[2].str());

            // Add the first integer to vector1 (after D)
            vector1.push_back(first_integer);
            // Add the second integer to vector2
            vector2.push_back(second_integer);

            // Move the search start position to the end of the current match
            search_start = match.suffix().first;
        }
    }

    const std::string fen = line.substr(0, semicolon_pos - 1);
    const ChessBoard board = ChessBoard::fromFEN(fen);

    assert(vector1.size() == vector2.size());
    for (int i = 0; i < vector1.size(); i++) {
        depth_t depth = vector1[i];
        perft_t expected = vector2[i];
        perft_t observed = perft(board, depth);
        if (verbose and expected == observed) {
            std::cout << "passed perft for " << fen << " at depth " << int(depth) << ". Nodes=" << expected << std::endl;
        }
        if (expected != observed) {
            std::cout << "FAILED perft for " << fen << " at depth " << int(depth) << ". Expected " << expected << " nodes. observed " << observed << " nodes." << std::endl;
            return false;
        }
    }
    return true;
}

void runPerftSuite(const std::string& filename, bool verbose) {
    bool passedAll = true;
    std::ifstream file(filename);
    std::string line;

    while (file.peek() != EOF) {
        getline(file, line);
        passedAll = runPerftSuiteLine(line, verbose) and passedAll;
    }

    if (passedAll)
        std::cout << "PASSED perft suite " << filename << std::endl;

    file.close();
}

void printZobrists() {
    std::mt19937_64 engine(1234567890);
    std::uniform_int_distribution<uint64_t> distr(0, std::numeric_limits<uint64_t>::max());

    // Step 1: Piece zobrists
    std::cout << std::hex;
    std::cout << "constexpr zobrist_t pieceZobrists[768] = {";
    for (int i = 0; i < 768; i++) {
        std::cout << "0x" << distr(engine) << "ULL,";
    }
    std::cout << "};" << std::endl;

    // Step 2: Rights zobrists
    std::cout << "constexpr zobrist_t rightsZobrists[256] = {";
    for (int i = 0; i < 256; i++) {
        std::cout << "0x" << distr(engine) << "ULL,";
    }
    std::cout << "};" << std::endl;

    // Step 3: stm zobrist
    std::cout << "constexpr zobrist_t stmZobrist = 0x" << distr(engine) << "ULL;" << std::endl;
}

static uint64_t perfTTHits;
perft_t zobristPerft(const ChessBoard& board, depth_t depth, bool isRoot) {
    static std::unordered_map<zobrist_t, std::string> perfTT;

    if (isRoot)
        perfTT.clear();

    board.areBitboardsCorrect();

    // Step 0: Probe TT
    auto ttResult = perfTT.find(board.getZobristCode());
    bool found = ttResult != perfTT.end();

    // Step 0.5: Check that the fens from probing TT are equal
    if (found) {
        perfTTHits++;
        // std::cout << "perfTT hit" << std::endl;
        if (perfTT[board.getZobristCode()] != board.toFEN()) {
            std::cout << "FAILED zobristPerft: Zobrist code collision" << std::endl;
            std::cout << "zobrist code is " << board.getZobristCode() << std::endl;
            std::cout << "FEN from TT is " << perfTT[board.getZobristCode()] << std::endl;
            std::cout << "FEN from board is " << board.toFEN() << std::endl;
            std::cout << std::endl;
        }
    }

    // Step 1: Return 1 at leaf nodes
    if (depth <= 0)
        return 1;

    // Step 2: Step through all the moves, incrementing count each time
    MoveList moves = board.getPseudoLegalMoves();
    perft_t count = 0;
    for (move_t move : moves) {
        ChessBoard newBoard = board;
        newBoard.makemove(move);
        bool canKingBeTaken;
        if (mvs::isCastle(move)) {
            ChessBoard slideBoard = board;
            slideBoard.makemove(mvs::castleToKingSlide(move));
            canKingBeTaken = slideBoard.canTheKingBeTaken() or newBoard.canTheKingBeTaken();
        }
        else
            canKingBeTaken = newBoard.canTheKingBeTaken();
        if (!canKingBeTaken)
            count += zobristPerft(newBoard, depth_t(depth - 1), false);
    }

    // Step 3: Put the result in the TT
    if (!found)
        perfTT[board.getZobristCode()] = board.toFEN();

    // Step 4: Return the count
    return count;
}

// ChatGPT generated this code
bool runZobristPerftSuiteLine(const std::string& line, bool verbose) {
    std::vector<int> vector1;  // Stores the values after D
    std::vector<perft_t> vector2;  // Stores the other integers

    // Extract the part after the semicolon
    size_t semicolon_pos = line.find(';');
    if (semicolon_pos != std::string::npos) {
        std::string pairs = line.substr(semicolon_pos + 1);

        // Regex to match D<int> <integer> pairs
        std::regex pair_regex(R"(D(\d+)\s+(\d+))");
        std::smatch match;
        std::string::const_iterator search_start = pairs.cbegin();

        // Loop over all matches
        while (std::regex_search(search_start, pairs.cend(), match, pair_regex)) {
            // Extract the pair of integers
            int first_integer = std::stoi(match[1].str());
            int second_integer = std::stoi(match[2].str());

            // Add the first integer to vector1 (after D)
            vector1.push_back(first_integer);
            // Add the second integer to vector2
            vector2.push_back(second_integer);

            // Move the search start position to the end of the current match
            search_start = match.suffix().first;
        }
    }

    const std::string fen = line.substr(0, semicolon_pos - 1);
    const ChessBoard board = ChessBoard::fromFEN(fen);

    assert(vector1.size() == vector2.size());
    for (int i = 0; i < vector1.size(); i++) {
        depth_t depth = vector1[i];
        perft_t expected = vector2[i];
        perft_t observed = zobristPerft(board, depth, true);
        if (verbose and expected == observed) {
            std::cout << "passed perft for " << fen << " at depth " << int(depth) << ". Nodes=" << expected << std::endl;
        }
        if (expected != observed) {
            std::cout << "FAILED perft for " << fen << " at depth " << int(depth) << ". Expected " << expected << " nodes. observed " << observed << " nodes." << std::endl;
            return false;
        }
    }

    std::cout << "total perfTT hits: " << perfTTHits << std::endl;

    return true;
}

void runZobristPerftSuite(const std::string& filename, bool verbose) {
    bool passedAll = true;
    std::ifstream file(filename);
    std::string line;

    while (file.peek() != EOF) {
        getline(file, line);
        passedAll = runZobristPerftSuiteLine(line, verbose) and passedAll;
    }

    if (passedAll)
        std::cout << "PASSED zobrist perft suite " << filename << std::endl;

    file.close();
}

void runEvalTestSuite(const std::string& bookFilename, const std::string& evalsFilename) {
    std::vector<ChessBoard> boards;
    std::ifstream bookFile(bookFilename);
    std::string fen;
    while (bookFile.peek() != EOF) {
        getline(bookFile, fen);
        boards.push_back(ChessBoard::fromFEN(fen));
    }

    std::vector<eval_t> evals;
    std::ifstream evalsFile(evalsFilename);
    while (evalsFile.peek() != EOF) {
        eval_t eval = 0;
        evalsFile >> eval;
        evals.push_back(eval);
    }

    bool passed = true;

    if (evals.size() != boards.size()) {
        std::cout
                << "FAILED runEvalTestSuite: boards and expected evals are not the same length. Check that you have the correct files."
                << std::endl;
        passed = false;
    }

    int numPassed = 0;
    int numFailed = 0;

    if (passed) {
        for (int i = 0; i < evals.size(); i++) {
            ChessBoard board = boards[i];
            eval_t eval = board.getEval();
            eval_t whiteRelativeEval = board.getSTM() == sides::WHITE ? eval : -eval;

            if (whiteRelativeEval != evals[i]) {
                std::cout << "FAILED eval test suite: evals don't match" << std::endl;
                std::cout << "FEN is " << board.toFEN() << std::endl;
                std::cout << "Eval from tuner is " << evals[i] << std::endl;
                std::cout << "Eval from board is " << whiteRelativeEval << std::endl;
                std::cout << std::endl;
                passed = false;
                numFailed++;
            }
            else {
                numPassed++;
            }
        }
    }

    std::cout << "eval test suite: passed " << numPassed << ", failed " << numFailed << std::endl;

    if (numPassed == 0 and numFailed == 0) {
        std:: cout << "FAILED eval test suite: could not find file" << std::endl;
    }
    else if (passed) {
        std::cout << "PASSED eval test suite" << std::endl;
    }
}

void runEasyPuzzleTestSuite(const std::string& bookFilename, const std::string& bestMovesFilename) {
    std::vector<ChessBoard> boards;
    std::ifstream bookFile(bookFilename);
    std::string fen;
    while (bookFile.peek() != EOF) {
        getline(bookFile, fen);
        boards.push_back(ChessBoard::fromFEN(fen));
    }

    std::vector<std::string> bestMoves;
    std::ifstream bestMovesFile(bestMovesFilename);
    std::string bestMove;
    while (bestMovesFile.peek() != EOF) {
        getline(bestMovesFile, bestMove);
        bestMoves.push_back(bestMove);
    }

    bool passed = true;

    if (bestMoves.size() != boards.size()) {
        std::cout
                << "FAILED runEasyPuzzleTestSuite: boards and expected best moves are not the same length. Check that you have the correct files."
                << std::endl;
        std::cout << "bestMoves.size() is " << bestMoves.size() << std::endl;
        std::cout << "boards.size() is " << boards.size() << std::endl;
        passed = false;
    }

    int numPassed = 0;
    int numFailed = 0;

    if (passed) {
        for (int i = 0; i < boards.size(); i++) {
            ChessBoard board = boards[i];
            sg::ThreadData data = rootSearch(board);
            std::string searchBestMove = moveToLAN(data.rootBestMove);
            bestMove = bestMoves[i];

            if (bestMove != searchBestMove) {
                std::cout << "FAILED easy puzzle test: engine did not play the best move" << std::endl;
                std::cout << "FEN is " << board.toFEN() << std::endl;
                std::cout << "Search best move is " << searchBestMove << std::endl;
                std::cout << "Expected best move is " << bestMove << std::endl;
                std::cout << std::endl;
                passed = false;
                numFailed++;
            }
            else {
                numPassed++;
            } // end else
        } // end for loop over boards.size()
    } // end if passed

    if (numPassed == 0 and numFailed == 0) {
        std:: cout << "FAILED easy puzzle test suite: could not find file" << std::endl;
    }
    else if (passed) {
        std::cout << "PASSED easy puzzle test suite" << std::endl;
    }
} // end runEasyPuzzleTestSuite

void printFenMoveOrder(const std::string& fen) {
    ChessBoard board = ChessBoard::fromFEN(fen);
    MoveList rawMoves = board.getPseudoLegalMoves();
    MoveList moves;
    for (move_t move: rawMoves) {
        if (mvs::isTactical(move))
            moves.push_back(move);
    }

    scoreMovesByMVVLVA(moves);
    std::sort(moves.begin(), moves.end(), std::greater<move_t>());

    for (move_t move: rawMoves) {
        if (mvs::isQuiet(move))
            moves.push_back(move);
    }

    for (move_t move: moves) {
        std::cout << moveToLAN(move);
        if (mvs::isCapture(move))
            std::cout << " " << getMVVLVAScore(move);
        std::cout << std::endl;
    }
}

void manualTTTest() {
    TT tt;
    tt.put(1234567890ULL, mvs::constructMove(9,11,flags::DOUBLE_PAWN_PUSH_FLAG, pcs::PAWN, pcs::PAWN), 100, 1, 6);

    tt.put(809765213ULL, mvs::constructMove(squares::e8,squares::c8,flags::LONG_CASTLE_FLAG, pcs::KING, 0), -349, 3, 9); // Index collision

    TTEntry e1 = tt.get(1234567890ULL); // tt hit
//    TTEntry e1 = tt.get(809765213ULL); // other tt hit
//    TTEntry e1 = tt.get(1234567891ULL); // index collision
//    TTEntry e1 = tt.get(0); // other index collision
//    TTEntry e1 = tt.get(~0ULL); // different (empty) bucket
    std::cout << "zobristCode is " << e1.zobristCode << std::endl;
    std::cout << "eval is " << e1.score << std::endl;
    std::cout << "ttMove is " << e1.ttMove << std::endl;
    std::cout << "ttFlag is " << int(e1.ttFlag) << std::endl;
    std::cout << "depth is " << int(e1.depth) << std::endl;
}

void printSEEOfMoves(const std::string& fen) {
    ChessBoard board = ChessBoard::fromFEN(fen);
    std::cout << std::boolalpha;

    for (move_t move : board.getPseudoLegalMoves()) {
        if (board.isLegal(move)) {
            std::cout << moveToLAN(move) << " " << board.isGoodSEE(move) << std::endl;
        } // end if move is legal
    } // end for loop over pseudolegal moves
} // end printSEEOfMoves function definition

void nullMoveTests() {
    // Test 1: Null move in normal position
    // Test 2: Null move when en passant is possible
    // Test 3: Null move when halfmove is 99

    ChessBoard board1 = ChessBoard::fromFEN("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1");
    board1.makeNullMove();
    std::cout << board1.toFEN() << std::endl;

    board1 = ChessBoard::fromFEN("8/8/K2p4/1Pp4r/1R3p1k/8/4P1P1/8 w - c6 0 2");
    board1.makeNullMove();
    std::cout << board1.toFEN() << std::endl;

    board1 = ChessBoard::fromFEN("8/2p5/K2p4/1P5r/1R2Pp2/6k1/6P1/8 b - e3 0 2");
    board1.makeNullMove();
    std::cout << board1.toFEN() << std::endl;

    board1 = ChessBoard::fromFEN("8/2p5/K2p4/1P5r/1R2Pp2/6k1/6P1/8 b - e3 99 2");
    board1.makeNullMove();
    std::cout << board1.toFEN() << std::endl;
}

void canTryNMPTests() {
    std::cout << std::boolalpha;

    // Test 0: startpos
    ChessBoard board = ChessBoard::startpos();
    std::cout << board.canTryNMP() << std::endl; // should be true

    // Test 1: White in check
    board = ChessBoard::fromFEN("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1");
    std::cout << board.canTryNMP() << std::endl; // should be false

    // Test 2: Black in check
    board = ChessBoard::fromFEN("r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1");
    std::cout << board.canTryNMP() << std::endl; // should be false

    // Test 3: White to move, both players have only king and pawns
    board = ChessBoard::fromFEN("4k3/pppppppp/8/8/8/8/PPPPPPPP/4K3 w - - 0 1");
    std::cout << board.canTryNMP() << std::endl; // should be false

    // Test 4: Black to move, both players have only king and pawns
    board = ChessBoard::fromFEN("4k3/pppppppp/8/8/8/8/PPPPPPPP/4K3 b - - 0 1");
    std::cout << board.canTryNMP() << std::endl; // should be false

    // Test 5: White to move, white has king and pawns only, but black has other pieces
    board = ChessBoard::fromFEN("4k3/pppppppp/3r4/8/8/8/PPPPPPPP/4K3 w - - 0 1");
    std::cout << board.canTryNMP() << std::endl; // should be false

    // Test 6: Black to move, white has king and pawns only, but black has other pieces
    board = ChessBoard::fromFEN("4k3/pppppppp/3r4/8/8/8/PPPPPPPP/4K3 b - - 0 1");
    std::cout << board.canTryNMP() << std::endl; // should be true

    // Test 6: Black to move, black has king and pawns only, but white has other pieces
    board = ChessBoard::fromFEN("4k3/pppppppp/3R4/8/8/8/PPPPPPPP/4K3 b - - 0 1");
    std::cout << board.canTryNMP() << std::endl; // should be false

    // Test 7: White to move, black has king and pawns only but white has other pieces
    board = ChessBoard::fromFEN("4k3/pppppppp/3R4/8/8/8/PPPPPPPP/4K3 w - - 0 1");
    std::cout << board.canTryNMP() << std::endl; // should be true
}

void stagedMovegenKiwipeteTest() {
    ChessBoard board = ChessBoard::fromFEN("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
    MoveGenerator generator(sg::ThreadData(), board, 0);
    std::cout << "KIWIPETE STAGED MOVEGEN MOVE ORDER: " << std::endl;
    while (move_t move = generator.nextMove()) {
        std::cout << moveToLAN(move) << std::endl;
    }
}

int main() {
    std::cout << "Hello, World!" << std::endl;
//    runAllMovesTests();
//    runAllMoveConstructorTests();
//    fenTestSuite("8moves_v3.epd"); // This passed
//    fenTestSuite("Pohl.epd"); // This passed
//    splitperft("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -",1);
//    fenTestSuite("lichess-big3-scrubbed.epd"); // This passed
//    startposPerft(6); // This looks good
    runPerftSuite("standard.epd", true);
//    runZobristPerftSuite("standard.epd", true); // This only works if we stop printing halfmove and fullmove in fens
//    runEvalTestSuite("tiny_tests.txt", "expected_eval.txt");
//    runEasyPuzzleTestSuite("easy_puzzles.txt", "easy_puzzle_answers.txt");
//    printFenMoveOrder("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"); // kiwipete
//    printFenMoveOrder("r1b1k2r/pPpp1p2/8/2b1p1pp/2Qnn1P1/2PP1N2/1qN1PPBP/R1B1K2R w KQkq - 0 14");
//    manualTTTest();
//    printSEEOfMoves("rnb1k1nr/4qpp1/2p5/p3p3/2N3PN/1p1Q4/PPP1PPBR/2K4R b kq - 0 16");
//    nullMoveTests();
//    canTryNMPTests();
//    stagedMovegenKiwipeteTest();
    return 0;
}