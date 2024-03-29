#include "QuietHistory.h"
void QuietHistory::sortMovesByCutoffs(std::vector<move_t> &vec, unsigned int startIndex, unsigned int endIndex) const {
    // Recursive base case
    if (startIndex >= endIndex) {
        return;
    }

    // Choose a partition element
    move_t partition = vec[startIndex];

    // Loop through vec from startIndex to endIndex
    // Keep track of where the > partition elements start
    unsigned int i;
    unsigned int largerElementIndex = startIndex+1;
    move_t temp;
    for (i = startIndex+1; i <= endIndex; ++i) {
        if (lookupMoveCutoffCount(vec[i]) >= lookupMoveCutoffCount(partition)) {
            // Swap the larger/equal item to the left of the larger items
            // This is modified so the moves are going from highest to lowest reward, instead of lowest to highest.
            temp = vec[i];
            vec[i] = vec[largerElementIndex];
            vec[largerElementIndex] = temp;
            // Update largerElementIndex
            ++largerElementIndex;
        }

    }
    // Swap the partition element into place
    temp = vec[startIndex];
    vec[startIndex] = vec[largerElementIndex-1];
    vec[largerElementIndex-1] = temp;

    // Uncomment this line if you want to see each iteration
    //printVec(vec);

    // Recursive calls for two halves
    sortMovesByCutoffs(vec, startIndex, largerElementIndex-2);
    sortMovesByCutoffs(vec, largerElementIndex, endIndex);
}

QuietHistory::QuietHistory() =default;
void QuietHistory::recordKillerMove (move_t move) {
    cutoffCounts[move >> 4] ++;
}

void QuietHistory::sortMoves(std::vector<move_t>& quietMoves) const {
    sortMovesByCutoffs(quietMoves, 0, (quietMoves.size()) - 1);
}

void QuietHistory::sortMovesByCutoffs(std::array<move_t,218> &array, unsigned int startIndex, unsigned int endIndex) const {
    // Recursive base case
    if (startIndex >= endIndex) {
        return;
    }

    // Choose a partition element
    move_t partition = array[startIndex];

    // Loop through array from startIndex to endIndex
    // Keep track of where the > partition elements start
    unsigned int i;
    unsigned int largerElementIndex = startIndex+1;
    move_t temp;
    for (i = startIndex+1; i <= endIndex; ++i) {
        if (lookupMoveCutoffCount(array[i]) >= lookupMoveCutoffCount(partition)) {
            // Swap the larger/equal item to the left of the larger items
            // This is modified so the moves are going from highest to lowest reward, instead of lowest to highest.
            temp = array[i];
            array[i] = array[largerElementIndex];
            array[largerElementIndex] = temp;
            // Update largerElementIndex
            ++largerElementIndex;
        }
    }
    // Swap the partition element into place
    temp = array[startIndex];
    array[startIndex] = array[largerElementIndex - 1];
    array[largerElementIndex - 1] = temp;

    // Uncomment this line if you want to see each iteration
    //printVec(array);

    // Recursive calls for two halves
    sortMovesByCutoffs(array, startIndex, largerElementIndex - 2);
    sortMovesByCutoffs(array, largerElementIndex, endIndex);
}