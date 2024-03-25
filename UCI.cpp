#include "UCI.h"
#include <string>
#include <sstream>
#include <iostream>
#include <future>
#include <thread>
#include "search/RepetitionTable.h"
#include "search/Negamax.h"
#include "Eval.h"
#include "ChessGame.h"
using namespace std;
// There are still some strange multithreading behaviors that I can't explain or reproduce

static mutex mut;
static bool hasQuit = false;
void mutexPrint (string line) {
    if (!hasQuit) {
        mut.lock();
        cout << line << endl;
        mut.unlock();
    }
}

void setPointerTrueLater (const future<void>* fut, bool* ptr, const long long ms) {
    assert(fut->valid());
    fut->wait_for(std::chrono::milliseconds(ms));
    *ptr = true;
}

void uciSearch (const ChessGame* game, promise<void>* pr, const future<void>* fut, const long long ms) {
    // Set up the board
    ChessBoard simpleBoard = game->getCurrentPosition();
    string fenNotation = simpleBoard.toFenNotation();
    ChessBoard board = ChessBoard::boardFromFENNotation(fenNotation);

    // Set up search parameters
    int depth;
    float eval = board.getNegaStaticEval();
    vector<uint16_t> moves;
    board.getLegalMoves(moves);
    uint16_t bestMoveFromPrevious = moves.at(0);
    assert(!game->hasGameEnded());
    RepetitionTable repetitionTable(*game);

    // Set up the sleep thread
    bool* isSearchCancelled = new bool(false);
    std::thread* sleepThread = new std::thread(setPointerTrueLater,fut,isSearchCancelled,ms);
    search::NegamaxData data(isSearchCancelled,repetitionTable,1);
    for (depth = 1; depth < 100; depth++) {
        try {
            u_int16_t bestMove = bestMoveFromPrevious;
            data.extendKillersToDepth(depth);
            search::getNegamaxBestMoveAndEval(board,depth,data,eval,bestMove,eval);
            if (bestMove != SEARCH_FAILED_MOVE_CODE)
                bestMoveFromPrevious = bestMove;
            if (eval == MATE_VALUE) // mate pruning. We find the fastest mate.
                break;
        }
        catch (const search::SearchCancelledException& e) {
            break;
        }
    } // end for loop over depth

    // Once we have broken out of the ID loop, one of three things happened
    // 1) we ran out of time. isSearchCancelled is true and not in use. sleepThread has exited.
    // 2) we received a stop command from the GUI. isSearchCancelled is true and not in use. sleepThread has exited.
    // 3) we exited the loop because we are doing mate pruning. sleepThread has not exited. isSearchCancelled is false and in use.

    // In cases 1 and 2, we don't need to do anything
    // This deals with case 3
    if (!*isSearchCancelled)
        pr->set_value();

    // In all 3 cases, sleepThread will be done, so joining it won't wait for any more time
    sleepThread->join();
    delete sleepThread;
    delete isSearchCancelled;

    // Print out the best move because this is the UCI
    // Whether we ran out of time, received a "stop" command, or triggered mate distance pruning
    mutexPrint("bestmove " + ChessBoard::moveToPureAlgebraicNotation(bestMoveFromPrevious));
} // end uciSearch

void uciLoop () {
    using namespace std;
    string command;
    ChessBoard position;
    ChessGame game;
    thread* searchThread = nullptr;
    promise<void>* pr = nullptr;
    future<void>* fut = nullptr;

    while (true) {
        getline(cin,command);
        if (command == "uci") {
            mutexPrint("id name Amethyst");
            mutexPrint("id author Noah Holbrook");
            mutexPrint("option name Hash type spin default 1 min 1 max 1");
            mutexPrint("option name Threads type spin default 1 min 1 max 1");
            mutexPrint("uciok");
        }
        else if (command == "isready") {
            mutexPrint("readyok");
        }
        else if (command == "ucinewgame") {
            // TODO: Implement this
        }
        else if (command.rfind("setoption",0) == 0) {
            // TODO: Implement this
        }
        else if (command.rfind("position",0) == 0) {
            stringstream ss(command);
            string word;
            // "position"
            ss >> word;
            assert(word == "position");
            // "startpos" or "fen"
            ss >> word;
            if (word == "startpos") {
                position = ChessBoard();
            }
            else if (word == "fen") {
                string parts[6] = {"","","","","",""};
                for (string& part: parts) {
                    ss >> part;
                }
                word = "";
                for (int i = 0; i < 6; i++) {
                    if (i == 5)
                        word += parts[i];
                    else
                        word += parts[i] + " ";
                }
//                cout << '"' << word << '"' << endl;
//                exit(43);
                position = ChessBoard::boardFromFENNotation(word);
            }
            else {
                assert(false); // This is as it should be
            }
            // "moves"
            ss >> word;
//            if (word != "moves")
//                position = ChessBoard();
            game = ChessGame(position);
            while (ss >> word) {
                game.makemove(game.getCurrentPosition().getMoveFromPureAlgebraicNotation(word));
            } // end while loop
        } // end if command starts with position
        else if (command.rfind("go",0) == 0) {
            if (command == "go infinite") {
                assert(false); // TODO: Implement this
            }
            else {
                int wtime = 0;
                int btime = 0;
                int winc = 0;
                int binc = 0;
                int movestogo = 0;
                int depth = 0;
                int nodes = 0;
                int mate = 0;
                int movetime = 0;
                stringstream ss(command);
                string word;
                while (ss >> word) {
                    if (word == "wtime")
                        ss >> wtime;
                    else if (word == "btime")
                        ss >> btime;
                    else if (word == "winc")
                        ss >> winc;
                    else if (word == "binc")
                        ss >> binc;
                    else if (word == "movestogo")
                        ss >> movestogo;
                    else if (word == "depth")
                        ss >> depth;
                    else if (word == "nodes")
                        ss >> nodes;
                    else if (word == "mate")
                        ss >> mate;
                    else if (word == "movetime")
                        ss >> movetime;
                } // end while ss >> word

                // Step 0: Determine the movetime
                if (movetime == 0) {
                    if (game.getCurrentPosition().getIsItWhiteToMove())
                        movetime = wtime / 40 + winc;
                    else
                        movetime = btime / 40 + binc;
                } // end if movetime == 0

                // Step 1: Clean up the old pointers. We don't know if they're null or not
                // The old search thread won't still be running; we will have received a "stop" command.
                // However, we need to free the memory used by the old pointers because they aren't freed anywhere else.
                if (searchThread != nullptr)
                    searchThread->join();
                delete searchThread;
                delete pr;
                delete fut;
                // Step 2: Reinitialize the promise and future pointers
                pr = new promise<void>();
                fut = new future(pr->get_future());
                // Step 3: Start the search thread!
                searchThread = new thread(uciSearch,&game,pr,fut,movetime);
            } // end else (command is not go infinite)
        } // end if command starts with go
        else if (command == "stop") {
            assert(searchThread != nullptr);
            assert(fut->valid());
            if (fut->wait_for(std::chrono::milliseconds(0)) == std::future_status::timeout)
                pr->set_value();
        }
        else if (command == "quit") {
            hasQuit = true;
            if (searchThread != nullptr) {
                assert(fut->valid());
                if (fut->wait_for(std::chrono::milliseconds(0)) == std::future_status::timeout)
                    pr->set_value();
                searchThread->join();
            }
            delete searchThread;
            delete pr;
            delete fut;
            exit(0);
        }
    } // end while true loop
} // end uciLoop