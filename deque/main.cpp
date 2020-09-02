#include <iostream>
#include <exception>
#include <queue>
#include <cstring>
#include <functional>
#include "gtest/gtest.h"

template <typename T>
class Deque{
private:
    T* data;
    int startIndex;
    uint length;
    uint capacity;
    uint sizeIncrement;
public:
    int backIndex();
    void pushFront(T x);
    void pushBack(T x);
    T popFront();
    T popBack();
    T peekFront();
    T peekBack();
    bool isEmpty();
    uint size();
    void growSize();
    Deque();
    ~Deque();
};

template <typename T>
int Deque<T>::backIndex(){
    return (startIndex + length - 1) % capacity;
}

template <typename T>
void Deque<T>::pushFront(T x){
    if (length == capacity - 2){
        growSize();
    }
    data[(startIndex -1) % capacity] = x;
    length++;
}

template <typename T>
void Deque<T>::pushBack(T x){
    if (length == capacity - 2){
        growSize();
    }
    data[(startIndex + length) % capacity] = x;
    length++;
}

template <typename T>
T Deque<T>::popFront(){
    if (length == 0){
        throw new std::logic_error("Popped from empty deque");
    }
    length--;
    startIndex = (startIndex + 1) % capacity;
    return data[startIndex - 1];
}

template <typename T>
T Deque<T>::popBack(){
    if (length == 0){
        throw new std::logic_error("Popped from empty deque");
    }
    length--;
    return data[startIndex + length];
}

template <typename T>
T Deque<T>::peekBack(){
    if (length == 0){
        throw new std::logic_error("Peeked from empty deque");
    }
    return data[startIndex + length - 1];
}

template <typename T>
T Deque<T>::peekFront(){
    if (length == 0){
        throw new std::logic_error("Peeked from empty deque");
    }
    return data[startIndex];
}

template <typename T>
bool Deque<T>::isEmpty(){
    return length == 0;
}

template <typename T>
uint Deque<T>::size(){
    return length;
}

template <typename T>
void Deque<T>::growSize(){
    capacity += sizeIncrement;
    T* newData = new T [capacity];
    std::memcpy(newData, data, sizeof(T) * size());
    delete [] data;
    data = newData;
    delete [] newData;
}

template <typename T>
Deque<T>::Deque()
: sizeIncrement{uint(1e3)},
capacity{uint(1e3)},
length{0},
startIndex{0}
{
    data = new T [capacity];
}

template <typename T>
Deque<T>::~Deque(){
    delete [] data;
}

TEST(TestDeque, emptyDequeIsEmpty){
    Deque<int> d;
    ASSERT_TRUE(d.isEmpty());
    ASSERT_EQ(d.size(), 0);
    ASSERT_THROW(d.peekFront(), std::logic_error);
    ASSERT_THROW(d.peekBack(), std::logic_error);
}

TEST(TestDeque, dequeHandlesOneElement){
    Deque<int> d;
    int el = 34;
    d.pushBack(el);
    
    ASSERT_EQ(d.peekBack(), el);
    ASSERT_EQ(d.peekFront(), el);
    ASSERT_FALSE(d.isEmpty());
    ASSERT_EQ(d.size(), 1);
    ASSERT_EQ(d.popFront(), el);
    
    d.pushFront(el);
    ASSERT_EQ(d.popBack(), el);
    
}

TEST(TestDeque, doubleSidedIntegerPushNPop){
    uint i = 0;
    uint upTo = 1e6;
    Deque<int> d;
    while(i < upTo){
        i++;
        d.pushBack(-i);
        d.pushFront(i);
    }
    i--;
    ASSERT_EQ(d.size() + 2, upTo * 2);
    while(i > 1){
        i--;
        ASSERT_EQ(d.peekBack(), -i);
        ASSERT_EQ(d.peekFront(), i);
        ASSERT_EQ(d.popBack(), -i);
        ASSERT_EQ(d.popFront(), i);
    }
    
}

TEST(TestDeque, forwardInchworm){
    Deque<int> d;
    for (int i = 0; i < 10; i++){
        d.pushBack(i);
    }
    ASSERT_EQ(d.size(), 10);
    for (int i = 0; i < 1e5; i++){
        ASSERT_EQ(d.popFront(), i % 10);
        ASSERT_EQ(d.peekFront(), (i + 1) % 10);
        ASSERT_EQ(d.peekBack(), (i - 1) % 10);
        d.pushBack(i % 10);
        ASSERT_EQ(d.size(), 10);
    }
}

TEST(TestDeque, backwardInchworm){
    Deque<int> d;
    for (int i = 0; i < 10; i++){
        d.pushFront(i);
    }
    ASSERT_EQ(d.size(), 10);
    for (int i = 0; i < 1e5; i++){
        ASSERT_EQ(d.popBack(), i % 10);
        ASSERT_EQ(d.peekBack(), (i + 1) % 10);
        ASSERT_EQ(d.peekFront(), (i - 1) % 10);
        d.pushFront(i % 10);
        ASSERT_EQ(d.size(), 10);
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
