#include <iostream>
#include <exception>
#include <queue>
#include <cstring>
#include <functional>
#include "gtest/gtest.h"

template <typename T>
class Deque{
private:
    size_t sizeIncrement;
    size_t capacity;
    T* data;
    size_t length;
    int startIndex;
public:
    int backIndex();
    void pushFront(T x);
    void pushBack(T x);
    T popFront();
    T popBack();
    T peekFront();
    T peekBack();
    bool isEmpty();
    size_t size();
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
    if (length == capacity - 1){
        growSize();
    }
    startIndex = (startIndex - 1 + capacity) % capacity;
    data[startIndex] = x;
    length++;
}

template <typename T>
void Deque<T>::pushBack(T x){
    if (length == capacity - 1){
        growSize();
    }
    data[(startIndex + length + capacity) % capacity] = x;
    length++;
}

template <typename T>
T Deque<T>::popFront(){
    if (length == 0){
        throw new std::logic_error("Popped from empty deque");
    }
    length--;
    startIndex = (startIndex + 1) % capacity;
    return data[(startIndex - 1 + capacity) % capacity];
}

template <typename T>
T Deque<T>::popBack(){
    if (length == 0){
        throw new std::logic_error("Popped from empty deque");
    }
    length--;
    return data[(startIndex + length) % capacity];
}

template <typename T>
T Deque<T>::peekBack(){
    if (length == 0){
        throw new std::logic_error("Peeked from empty deque");
    }
    return data[(startIndex + length - 1) % capacity];
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
size_t Deque<T>::size(){
    return length;
}

template <typename T>
void Deque<T>::growSize(){
    capacity += sizeIncrement;
    T* newData = new T [capacity];
//     std::memcpy(newData, data, sizeof data);
    for (size_t i = 0; i < capacity; i++){
        newData[i] = data[(startIndex + i) % capacity];
    }
    startIndex = 0;
    delete [] data;
    data = newData;
}

template <typename T>
Deque<T>::Deque()
    : sizeIncrement{size_t(1e3)},
      capacity{sizeIncrement},
      data{new T[capacity]},
      length{0},
      startIndex{0}
{}

template <typename T>
Deque<T>::~Deque(){
    delete [] data;
}

TEST(TestDeque, emptyDequeIsEmpty){
    Deque<int> d;
    ASSERT_TRUE(d.isEmpty());
    ASSERT_EQ(d.size(), 0);
    ASSERT_THROW(d.peekFront(), std::logic_error*);
    ASSERT_THROW(d.peekBack(), std::logic_error*);
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

TEST(TestDeque, dequeActsLikeStack){
    Deque<int> d;
    size_t i = 1;
    for (; i <= 2000; i++){
        d.pushFront(i);
    }
    i--;
    for (; i >= 1; i--){
        ASSERT_EQ(d.popFront(), i);
    }
}

TEST(TestDeque, doubleSidedIntegerPushNPop){
    int i = 0;
    size_t upTo = 1000;
    Deque<int> d;
    while(i < upTo){
        i++;
        d.pushBack(-i);
        d.pushFront(i);
    }
    i++;
    ASSERT_EQ(d.size(), upTo * 2);
    while(i > 2){
        i--;
        ASSERT_EQ(d.peekBack(), -i);
        ASSERT_EQ(d.peekFront(), i);
        ASSERT_EQ(d.popBack(), -i);
        ASSERT_EQ(d.popFront(), i);
//         std::cout << d.popBack() << "\n";
//         std::cout << d.popFront() << "\n";
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
        ASSERT_EQ(d.peekFront(), (i + 11) % 10);
        ASSERT_EQ(d.peekBack(), (i + 9) % 10);
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
        ASSERT_EQ(d.peekBack(), (i + 11) % 10);
        ASSERT_EQ(d.peekFront(), (i + 9) % 10);
        d.pushFront(i % 10);
        ASSERT_EQ(d.size(), 10);
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
