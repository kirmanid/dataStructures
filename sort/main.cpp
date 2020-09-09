#include <iostream>
#include <exception>
#include <queue>
#include <cstring>
#include <functional>
#include "gtest/gtest.h"

// mergeSort in ascending order
template<typename T>
T* mergeSort(T* unsorted, size_t length){
    if (length <= 1){
        return unsorted;
    }
    size_t aIndex, bIndex, aSize, bSize = 0;
    aSize = length/2;
    bSize = length - aSize;
    T pileA[aSize];
    T pileB[bSize];
    T sorted[aSize + bSize];
    
    
    for(size_t i = 0; i < length; i++){
        if (i < aSize){
            pileA[i] = unsorted[i];
        } else {
            pileB[i - aSize] = unsorted[i];
        }
    }
    pileA = mergeSort(pileA, aSize);
    pileB = mergeSort(pileB, bSize);
    
    
    while (aIndex + bIndex < aSize + bSize - 2){
        if (pileA[aIndex] > pileB[bIndex]){
            sorted[aIndex + bIndex] = pileA[aIndex];
            aIndex++;
        } else {
            sorted[aIndex + bIndex] = pileB[bIndex];
            bIndex++;
        }
        if (bIndex == bSize){
            sorted[aIndex + bIndex] = pileA[aIndex];
        }
        if (aIndex == aSize){
            sorted[aIndex + bIndex] = pileB[bIndex];
        }
    }
    return sorted;
}   

int main(int argc, char **argv) {
    int f [9] = {1,9,2,8,3,7,4,6,5};
    int* sorted = mergeSort<int>(f,9);
    for (size_t i = 0; i < 9; i++){
        std::cout << sorted[i] << std::endl;
    }
}
