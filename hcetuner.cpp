#include "chessboard.h"
#include "logarithm.h"
#include "attacks.h"

#include <iostream>
#include <fstream>
#include <sstream>

const std::string bookFilename = "lichess-big3-resolved.book";

std::vector<ChessBoard> readBook(const int length) {
    // Step 1: Initialize stuff
    std::vector<ChessBoard> book;
    std::ifstream file(bookFilename);
    std::string line;

    // Step 2: Read in one position for each line in the file
    for (int i = 0; i < length; i++) {
        if (file.peek() == EOF) {
            if (i == 0)
                std::cout << "Error in readBook: file is empty";
            else
                std::cout << "Error in readBook: reached end of file before book length limit was reached" << std::endl;
            exit(1);
        }
        getline(file, line);
        book.push_back(ChessBoard::fromFEN(line));
    }

    // Step 3: Close the file and return
    file.close();
    return book;
}

extern "C" void getKingSquares (const int side, int arr[], const int length) {
    std::vector<ChessBoard> boards = readBook(length);

    for (int i = 0; i < length; i++) {
        // Step 1: Get the square
        const ChessBoard board = boards.at(i);
        const bitboard_t kingBB = board.getSideBB(side) & board.getPieceBB(pcs::KING);
        square_t square = log2ll(kingBB);

        // Step 2: Annoying processing with the square
        // We have to flip the square vertically if the side is black
        square ^= side * 7;
        // We also have to switch from a8=7 to h1=7
        // This is done for PST interpretability and compatibility with python-chess
        const square_t file = squares::getFile(square);
        const square_t rank = squares::getRank(square);
        square = squares::squareFromFileRank(rank, file); // Yes this is supposed to pass in arguments "backwards"

        // Step 3: Put the square in the array
        arr[i] = square;
    }
}

extern "C" void getMobility(const int side, int arr[], const int length) {
    // NOTE: The array is implicitly 2D
    // So it is length*6
    // So arr[] is length 600 if length is 100
    std::vector<ChessBoard> boards = readBook(length);
    for (int i = 0; i < length; i++) {
        const ChessBoard board = boards.at(i);
        const bitboard_t allPieces = board.getSideBB(sides::WHITE) | board.getSideBB(sides::BLACK);
        const bitboard_t notFriendlyPieces = ~board.getSideBB(side);
        for (piece_t piece = pcs::PAWN; piece <= pcs::KING; piece++) {
            const int index = i * 6 + piece;
            bitboard_t remainingPieces = board.getPieceBB(piece) & board.getSideBB(side);
            bitboard_t squareBB;
            square_t square;
            while (remainingPieces) {
                squareBB = remainingPieces & -remainingPieces;
                remainingPieces -= squareBB;
                square = log2ll(squareBB);
                const bitboard_t attacks = getAttackedSquares(square, piece, allPieces, side);
                arr[index] += std::popcount(attacks & notFriendlyPieces);
            } // end while remainingPieces
        } // end for loop over piece
    } // end for loop over i
} // end getMobility

extern "C" void getPSTs(const int side, const int piece, const int maxPieces, int arr[], const int length) {
    std::vector<ChessBoard> boards = readBook(length);
    for (int i = 0; i < length; i++) {
        const ChessBoard board = boards.at(i);
        bitboard_t remainingPieces = board.getPieceBB(piece) & board.getSideBB(side);
        bitboard_t squareBB;
        square_t square;
        int index = i * maxPieces;
        while (remainingPieces) {
            // Step 1: Find the square
            squareBB = remainingPieces & -remainingPieces;
            remainingPieces -= squareBB;
            square = log2ll(squareBB);

            // Step 2: Annoying processing with the square
            // We have to flip the square vertically if the side is black
            square ^= side * 7;
            // We also have to switch from a8=7 to h1=7
            // This is done for PST interpretability and compatibility with python-chess
            const square_t file = squares::getFile(square);
            const square_t rank = squares::getRank(square);
            square = squares::squareFromFileRank(rank, file); // Yes this is supposed to pass in arguments "backwards"

            // Step 3: Write the square to the array
            arr[index++] = square;
        } // end while remainingPieces

        while (index < maxPieces * (i + 1)) {
            arr[index++] = 64; // 64 is the code for "this piece is not on the board"
        }
    } // end for loop over i
}

int get_phase(const std::string& fen) {
    // This function is taken from Gedas's texel tuner
    // https://github.com/GediminasMasaitis/texel-tuner/tree/main
    // and lightly modified

    int phase = 0;
    bool stop = false;
    for (const char ch : fen) {
        if (stop) {
            break;
        }

        switch (ch) {
            case 'n':
            case 'N':
                phase += 1;
                break;
            case 'b':
            case 'B':
                phase += 1;
                break;
            case 'r':
            case 'R':
                phase += 2;
                break;
            case 'q':
            case 'Q':
                phase += 4;
                break;
            case 'p':
            case '/':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
                break;
            case ' ':
                stop = true;
                break;
            default:
                break;
        } // end switch
    } // end for loop over characters in fen
    return std::min(phase, 24);
} // end get_phase method

extern "C" void getPhases(int arr[], const int length) {
    // Step 1: Initialize stuff
    std::ifstream file(bookFilename);
    std::string line;

    // Step 2: Read in one position and compute phase for each line in the file
    // NOTE: We are computing phases from the fens directly, without parsing to a ChessBoard
    for (int i = 0; i < length; i++) {
        if (file.peek() == EOF) {
            if (i == 0)
                std::cout << "Error in getPhases: file is empty";
            else
                std::cout << "Error in getPhases: reached end of file before book length limit was reached" << std::endl;
            exit(1);
        }
        getline(file, line);
        arr[i] = get_phase(line);
    }

    // Step 3: Close the file
    file.close();
}

extern "C" void getResults(float arr[], const int length) {
    // Step 1: Initialize stuff
    std::ifstream file(bookFilename);
    std::string line;

    // Step 2: Read in each line
    for (int i = 0; i < length; i++) {
        if (file.peek() == EOF) {
            if (i == 0)
                std::cout << "Error in getPhases: file is empty";
            else
                std::cout << "Error in getPhases: reached end of file before book length limit was reached" << std::endl;
            exit(1);
        }
        getline(file, line);
        std::stringstream ss(line);
        getline(ss,line,'[');
        float result = -100; // If it stays -100 we know we did something wrong
        ss >> result;
        arr[i] = result;
    }

    // Step 3: Close the file
    file.close();
}