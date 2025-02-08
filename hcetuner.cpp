#include "chessboard.h"
#include "logarithm.h"

#include <iostream>
#include <fstream>

const std::string bookFilename = "lichess-big3-resolved.book"; // WATCH OUT, THIS HAS EVALS IN THE LINES

std::vector<ChessBoard> readBook(const int length) {
    // Step 1: Initialize stuff
    std::vector<ChessBoard> book;
    std::ifstream file(bookFilename);
    std::string line;

    // Step 2: Read in one position for each line in the file
    for (int i = 0; i < length; i++) {
        if (file.peek() == EOF) {
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

extern "C" void getKingSquares (const side_t side, int arr[], const int length) {
    std::vector<ChessBoard> boards = readBook(length);

    for (int i = 0; i < length; i++) {
        const ChessBoard board = boards.at(i);
        const bitboard_t kingBB = board.getSideBB(side) & board.getPieceBB(pcs::KING);
        const square_t kingSquare = log2ll(kingBB);
        arr[i] = kingSquare;
    }
}

void add_fifthy (int arr[], int length) {
    for (int i=0; i < length; i++) {
        arr[i] = arr[i] + 50;
    }
}