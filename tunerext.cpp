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