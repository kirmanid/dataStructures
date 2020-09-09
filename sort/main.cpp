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
        T* sorted = new T[length];
        if (length == 1){
            sorted[0] = unsorted[0];
        }
        return sorted;
    }
    size_t aSize = length/2;
    size_t bSize = length - aSize;
    T pileA[aSize];
    T pileB[bSize];
    
    for(size_t i = 0; i < length; i++){
        if (i < aSize){
            pileA[i] = unsorted[i];
        } else {
            pileB[i - aSize] = unsorted[i];
        }
    }
    T* sortedPileA = mergeSort(pileA, aSize);
    T* sortedPileB = mergeSort(pileB, bSize);
    
    size_t aIndex = 0;
    size_t bIndex = 0;
    T* sorted =  new T[aSize + bSize];
    while (aIndex + bIndex < aSize + bSize){
        if (bIndex == bSize){
            sorted[aIndex + bIndex] = sortedPileA[aIndex];
            aIndex++;
            continue;
        }
        if (aIndex == aSize){
            sorted[aIndex + bIndex] = sortedPileB[bIndex];
            bIndex++;
            continue;
        }
        if (sortedPileA[aIndex] < sortedPileB[bIndex]){
            sorted[aIndex + bIndex] = sortedPileA[aIndex];
            aIndex++;
        } else {
            sorted[aIndex + bIndex] = sortedPileB[bIndex];
            bIndex++;
        }
    }
    delete [] sortedPileA;
    delete [] sortedPileB;
    return sorted;
}   

int main() {
    int f [9] = {1,9,2,8,3,7,4,6,5};
    int* sorted = mergeSort<int>(f,9);
    for (size_t i = 0; i < 9; i++){
        std::cout << sorted[i] << std::endl;
    }
}
