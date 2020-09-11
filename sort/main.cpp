#include <iostream>
#include <exception>
#include <queue>
#include <cstring>
#include <functional>
#include "gtest/gtest.h"

// mergeSort in ascending order
template<typename T>
T* mergeSortArray(T* unsorted, size_t length){
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

template <typename T>
void mergeSort(std::vector<T>& values){
    size_t length = values.size();
    if (length <= 1){
        return;
    }
    size_t aSize = length/2;
    size_t bSize = length - aSize;
    std::vector<T> pileA(aSize);
    std::vector<T> pileB(bSize);
    
    for(size_t i = 0; i < length; i++){
        if (i < aSize){
            pileA[i] = values[i];
        } else {
            pileB[i - aSize] = values[i];
        }
    }
        
    mergeSort(pileA);
    mergeSort(pileB);
    
    std::vector<T> sorted;
    
    size_t aIndex = 0;
    size_t bIndex = 0;
    size_t j = 0;
    while (aIndex + bIndex < aSize + bSize){
        if (bIndex == bSize){
            values[j++] = pileA[aIndex];
            aIndex++;
            continue;
        }
        if (aIndex == aSize){
            values[j++] = pileB[bIndex];
            bIndex++;
            continue;
        }
        if (pileA[aIndex] < pileB[bIndex]){
            values[j++] = pileA[aIndex];
            aIndex++;
        } else {
            values[j++] = pileB[bIndex];
            bIndex++;
        }
    }
}

template <typename T>
void quickSort(std::vector<T>& values){
    size_t length = values.size();
    if (length <= 1){
        return;
    }
    std::vector<T> pileA;
    std::vector<T> pileB;
    T pivot = values[length/2];
    
    for(size_t i = 0; i < length; i++){
        if (i == length/2){
            continue;
        }
        if (values[i] < pivot){
            pileA.push_back(values[i]);
        } else {
            pileB.push_back(values[i]);
        }
    }
        
    quickSort(pileA);
    quickSort(pileB);
    for (size_t i = 0; i < pileA.size(); i++){
        values[i] = pileA[i];
    }
    values[pileA.size()] = pivot;
    for (size_t i = 0; i < pileB.size(); i++){
        values[i + pileA.size() + 1] = pileB[i];
    }
}

template<typename T>
bool sortAndCheckN(size_t n){
    void* seed;
    srand(size_t(seed));
    std::vector<T> vector(n);
    for (size_t i = 0; i < n; i++){
        vector[i] = T(rand());
    }
    mergeSort<T>(vector); // assumes sort in place, ascending order
    bool sorted = true;
    for (size_t i = 1; i < n; i++){
        if (vector[i-1] > vector[i]){
            sorted = false;
        }
    }
    return sorted;
}

TEST(TestSort, oneElement){
    ASSERT_TRUE(sortAndCheckN<int>(1));
}

TEST(TestSort, twoElements){
    ASSERT_TRUE(sortAndCheckN<int>(2));
}

TEST(TestSort, manyElements){
    ASSERT_TRUE(sortAndCheckN<int>(2e5));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
