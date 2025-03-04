#include "uci.h"

#include <string>
#include <iostream>
#include <sstream>
#include <climits>
#include <algorithm>

#include "searchglobals.h"
#include "chessboard.h"
#include "search.h"
#include "bench.h"
#include "hce.h"


void uciLoop() {
    std::cout << "info string AMETHYST by Noah Holbrook" << std::endl;

    std::string command;
    ChessBoard position = ChessBoard::startpos();

    while (true) {
        getline(std::cin, command);
        if (command == "uci") {
            std::cout << "id name Amethyst" << std::endl;
            std::cout << "id author Noah Holbrook" << std::endl;
            std::cout << "option name Hash type spin default " << uciopt::HASH_DEFAULT << " min " << uciopt::HASH_MIN << " max " << uciopt::HASH_MAX << std::endl;
            std::cout << "option name Threads type spin default " << uciopt::THREADS_DEFAULT << " min " << uciopt::THREADS_MIN << " max " << uciopt::THREADS_MAX << std::endl;
            std::cout << "option name SyzygyPath type string default <empty>" << std::endl;
            std::cout << "option name UCI_ShowWDL type check default false" << std::endl;
            std::cout << "option name Move Overhead type spin default 10 min 0 max 5000" << std::endl;
            std::cout << "uciok" << std::endl;
        }

        else if (command == "isready") {
            std::cout << "readyok" << std::endl;
        }

        else if (command == "ucinewgame") {
            sg::GLOBAL_TT.clear();
        }

        else if (command.starts_with("setoption")) {
            if (command.starts_with("setoption name Hash value")) {
                std::stringstream ss(command);
                std::string word;
                for (int i = 0; i < 4; i++)
                    ss >> word;
                ss >> uciopt::HASH;
                uciopt::HASH = std::clamp(uciopt::HASH, uciopt::HASH_MIN, uciopt::HASH_MAX);
                std::cout << "info string uci option Hash has been set to " << uciopt::HASH << std::endl;
            }

            if (command.starts_with("setoption name Threads value")) {
                std::stringstream ss(command);
                std::string word;
                for (int i = 0; i < 4; i++)
                    ss >> word;
                ss >> uciopt::THREADS;
                uciopt::THREADS = std::clamp(uciopt::THREADS, uciopt::THREADS_MIN, uciopt::THREADS_MAX);
                std::cout << "info string uci option Threads has been set to " << uciopt::THREADS << std::endl;
            }
        }

        else if (command.starts_with("position")) {
            // Step 1: Clear the repetition tables
            sg::repetitionTables[sides::WHITE].clear();
            sg::repetitionTables[sides::BLACK].clear();

            // Step 2: Initialize needed variables
            std::stringstream ss(command);
            std::string word;
            position = ChessBoard::startpos();
            bool parsingMoves = false;

            // Step 3: Loop over the words in the string
            while (ss >> word) {
                if (parsingMoves) {
                    move_t move = position.parseLANMove(word);
                    if (mvs::isIrreversible(move)) {
                        sg::repetitionTables[sides::WHITE].clear();
                        sg::repetitionTables[sides::BLACK].clear();
                    }
                    else {
                        sg::repetitionTables[position.getSTM()].insert(position.getZobristCode());
                    }
                    position.makemove(move);
                }

                else if (word == "fen") {
                    std::string fen;
                    for (int i = 0; i < 6; i++) { // TODO: Handle 4-field fens
                        if (i)
                            fen += " ";
                        ss >> word;
                        fen += word;
                    } // end for loop over i
                    position = ChessBoard::fromFEN(fen);
                } // end if word == fen

                else if (word == "moves") {
                    parsingMoves = true;
                } // end if word == "moves"
            } // end while ss >> word

            // Step 4: Add the last position to the repetition table
            // We do this because when parsing moves, the position BEFORE the move was made was added to the repetition table
            sg::repetitionTables[position.getSTM()].insert(position.getZobristCode());
        } // end if command starts with position

        else if (command.starts_with("go")) {
            int wtime = 0;
            int btime = 0;
            int winc = 0;
            int binc = 0;
            int movestogo = 0;
            sg::depthLimit = 100;
            sg::nodesLimit = INT64_MAX;
            int mate = 0;
            int movetime = -1;
            std::stringstream ss(command);
            std::string word;
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
                else if (word == "depth") {
                    ss >> sg::depthLimit;
                    sg::depthLimit = std::clamp(sg::depthLimit, 1, 100);
                    sg::nodesLimit = INT64_MAX;
                    movetime = 1000000000;
                }
                else if (word == "nodes") {
                    ss >> sg::nodesLimit;
                    sg::depthLimit = 100;
                    movetime = 1000000000;
                }
                else if (word == "mate")
                    ss >> mate;
                else if (word == "movetime")
                    ss >> movetime;
                else if (word == "infinite")
                    movetime = 1000000000; // search for 1 million seconds
            } // end while ss >> word

            if (movetime >= 0) {
                sg::softTimeLimit = movetime;
                sg::hardTimeLimit = movetime;
            }
            else if (position.getSTM() == sides::WHITE) {
                sg::softTimeLimit = spsa::calcSoftTimeLimit(wtime, winc);
                sg::hardTimeLimit = spsa::calcHardTimeLimit(wtime, winc);
            }
            else {
                sg::softTimeLimit = spsa::calcSoftTimeLimit(btime, binc);
                sg::hardTimeLimit = spsa::calcHardTimeLimit(btime, binc);
            }

            rootSearch(position);

        } // end if command starts with go

        else if (command == "staticeval") {
            std::cout << hce::getStaticEval(position) << std::endl;
        }

        else if (command == "bench") {
            bench();
        }

        else if (command == "quit") {
            exit(0);
        }
    } // end while true
} // end uciLoop
