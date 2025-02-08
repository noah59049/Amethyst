#include "chessboard.h"
#include "logarithm.h"

#include <iostream>
#include <fstream>

const std::string bookFilename = "Pohl.epd";

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

int main() {
    int arr[30]{};

    getKingSquares(sides::WHITE, arr, 30);
    for (int num : arr) {
        std::cout << num << std::endl;
    }

    std::cout << "Hello, World!" << std::endl;
    return 0;
}