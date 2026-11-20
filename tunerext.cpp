/*
 * This file is meant to be compiled into a shared library (.dylib, .so, or .dll, depending on the OS)
 * It will be called by the python HCE tuner to get tensors of features.
 * This represents a significant speedup over using the python-chess library to compute features from a file of FENs.
 *
 * In order to avoid boilerplate, arrays are allocated in python, and C++ only modifies them.
 * Also, these C++ functions only deal with 1D arrays, even when the arrays are "actually" 2D
 * (for example, if you have 10000 fens, mobility is "actually" a tensor of size 10000*6,
 * but this treats it as a 1D array of size 60000.
 * The first 6 elements in the array represent the pawn, knight, bishop, rook, queen, king mobility for the first FEN, and so on.
 * It is a simple matter for python to reshape the array.
 * See hcetuner.cpp for the actual implementations of these functions.
 */


#include "hcetuner.h"

extern "C" void getKingSquares (const int side, int arr[], const int length) {
    getKingSquares_(side, arr, length);
}

extern "C" void getMobility(const int side, int arr[], const int length) {
    getMobility_(side, arr, length);
}

extern "C" void getPSTs(const int side, const int piece, const int maxPieces, int arr[], const int length) {
    getPSTs_(side, piece, maxPieces, arr, length);
}

extern "C" void getPhases(int arr[], const int length) {
    getPhases_(arr, length);
}

extern "C" void getResults(float arr[], const int length) {
    getResults_(arr, length);
}