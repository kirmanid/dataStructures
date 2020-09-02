#include <iostream>
#include <exception>
#include <queue>
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

int Deque::backIndex(){
    return (startIndex + length - 1) % capacity;
}

void Deque::pushFront(T x){
    if (length == capacity - 2){
        growSize();
    }
    data[(startIndex -1) % capacity] = x;
    length++;
}

void Deque::pushBack(T x){
    if (length == capacity - 2){
        growSize();
    }
    data[(startIndex + length) % capacity] = x;
    length++;
}

T Deque::popFront(){
    if (length == 0){
        throw new std::logic_error("Popped from empty deque");
    }
    length--;
    startIndex = (startIndex + 1) % capacity;
    return data[startIndex - 1];
}

T Deque::popBack(){
    if (length == 0){
        throw new std::logic_error("Popped from empty deque");
    }
    length--;
    return data[startIndex + length];
}

T Deque::peekBack(){
    if (length == 0){
        throw new std::logic_error("Peeked from empty deque");
    }
    return data[startIndex + length - 1];
}

T Deque::peekFront(){
    if (length == 0){
        throw new std::logic_error("Peeked from empty deque");
    }
    return data[startIndex];
}

bool Deque::isEmpty(){
    return length == 0;
}

uint Deque::size(){
    return length;
}

void Deque::growSize(){
    capacity += sizeIncrement;
    new T newData [capacity];
    std::memcpy(newData, data, sizeof(data));
    delete [] data;
    data = newData;
}

Deque::Deque()
: sizeIncrement{uint(1e3)},
capacity{uint(1e3)},
length{0},
startIndex{0}
{
    data = new T [capacity];
}

Deque::~Deque(){
    delete [] data;
}

TEST(TestDeque, emptyDequeIsEmpty){
    Deque d<int>;
    ASSERT_TRUE(d.isEmpty());
    ASSERT_EQ(d.size() == 0);
    ASSERT_THROW(s.peekFront(), std::logic_error);
    ASSERT_THROW(s.peekBack(), std::logic_error);
}

TEST(TestDeque, dequeHandlesOneElement){
    Deque d<int>;
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
    Deque d<int>;
    while(i < upTo){
        i++;
        d.pushBack(i);
        d.pushFront(i);
    }
    i--;
    ASSERT_EQ(d.size() + 2, upTo * 2);
    while(i > 1){
        i--;
        ASSERT_EQ(d.peekBack(), i);
        ASSERT_EQ(d.peekFront(), i);
        ASSERT_EQ(d.popBack(), i);
        ASSERT_EQ(d.popFront(), i);
    }
    
}

TEST(TestDeque, forwardInchworm){
    Deque d<int>;
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
    Deque d<int>;
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
