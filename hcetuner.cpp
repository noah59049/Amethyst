#include "chessboard.h"
#include "logarithm.h"
#include "attacks.h"

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