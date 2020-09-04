#include <iostream>
#include <exception>
#include <queue>
#include <cstring>
#include <functional>
#include "gtest/gtest.h"

template <typename T>
struct dllNode{
    dllNode* fwd;
    dllNode* back;
    T data;
};

template <typename T>
class Deque{
private:
    dllNode<T>* frontmost;
    dllNode<T>* backmost;
public:
    size_t length;
    void pushFront(T x);
    void pushBack(T x);
    T popFront();
    T popBack();
    T peekFront();
    T peekBack();
    bool isEmpty();
    size_t size();
    void incrementSize(int i); //////
    Deque();
    ~Deque();
};


template <typename T>
void Deque<T>::pushFront(T x){
    dllNode<T>* newFront = new dllNode<T>;
    newFront->data = x;
    frontmost->fwd = newFront;
    newFront->back = frontmost;
    frontmost = newFront;
    if (length == 0){
        backmost = frontmost;
    }
    length++;
}

template <typename T>
void Deque<T>::pushBack(T x){
    dllNode<T>* newBack = new dllNode<T>;
    newBack->data = x;
//     length++;
    if (isEmpty()){
        backmost = newBack;
        frontmost = backmost;
    } else {
        backmost->back = newBack;
        newBack->fwd = backmost;
        backmost = newBack;
    }
}

template <typename T>
T Deque<T>::popFront(){
    if (length == 0){
        throw new std::logic_error("Popped from empty deque");
    }
    length--;
    T returnVal = frontmost->data;
    frontmost = frontmost->back;
    delete frontmost->fwd;
    return returnVal;
}

template <typename T>
T Deque<T>::popBack(){
    if (length == 0){
        throw new std::logic_error("Popped from empty deque");
    }
    length--;
    T returnVal = backmost->data;
    backmost = backmost->fwd;
    delete backmost->back;
    return returnVal;
}

template <typename T>
T Deque<T>::peekBack(){
    if (length == 0){
        throw new std::logic_error("Peeked from empty deque");
    }
    return backmost->data;
}

template <typename T>
T Deque<T>::peekFront(){
    if (length == 0){
        throw new std::logic_error("Peeked from empty deque");
    }
    return frontmost->data;
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
void Deque<T>::incrementSize(int i){
    length += i;
}

template <typename T>
Deque<T>::Deque()
    : frontmost{nullptr},
    backmost{nullptr},
    length{0}
{}

template <typename T>
Deque<T>::~Deque(){
    while(length > 0){
        popFront();
    }
}

// TEST(TestDeque, emptyDequeIsEmpty){
//     Deque<int> d;
//     ASSERT_TRUE(d.isEmpty());
//     ASSERT_EQ(d.size(), 0);
//     ASSERT_THROW(d.peekFront(), std::logic_error*);
//     ASSERT_THROW(d.peekBack(), std::logic_error*);
// }
// 
// TEST(TestDeque, dequeHandlesOneElement){
//     Deque<int> d;
//     int el = 34;
//     d.pushBack(el);
//     
//     ASSERT_EQ(d.peekBack(), el);
//     ASSERT_EQ(d.peekFront(), el);
//     ASSERT_FALSE(d.isEmpty());
//     ASSERT_EQ(d.size(), 1);
//     ASSERT_EQ(d.popFront(), el);
//     
//     d.pushFront(el);
//     ASSERT_EQ(d.popBack(), el);
//     
// }
// 
// TEST(TestDeque, dequeActsLikeStack){
//     Deque<int> d;
//     size_t i = 1;
//     for (; i <= 2000; i++){
//         d.pushFront(i);
//     }
//     i--;
//     for (; i >= 1; i--){
//         ASSERT_EQ(d.popFront(), i);
//     }
// }
// 
// TEST(TestDeque, doubleSidedIntegerPushNPop){
//     int i = 0;
//     size_t upTo = 1000;
//     Deque<int> d;
//     while(i < upTo){
//         i++;
//         d.pushBack(-i);
//         d.pushFront(i);
//     }
//     i++;
//     ASSERT_EQ(d.size(), upTo * 2);
//     while(i > 2){
//         i--;
//         ASSERT_EQ(d.peekBack(), -i);
//         ASSERT_EQ(d.peekFront(), i);
//         ASSERT_EQ(d.popBack(), -i);
//         ASSERT_EQ(d.popFront(), i);
//         std::cout << d.popBack() << "\n";
//         std::cout << d.popFront() << "\n";
//     }
//     
// }
// 
// TEST(TestDeque, forwardInchworm){
//     Deque<int> d;
//     for (int i = 0; i < 10; i++){
//         d.pushBack(i);
//     }
//     ASSERT_EQ(d.size(), 10);
//     for (int i = 0; i < 1e5; i++){
//         ASSERT_EQ(d.popFront(), i % 10);
//         ASSERT_EQ(d.peekFront(), (i + 11) % 10);
//         ASSERT_EQ(d.peekBack(), (i + 9) % 10);
//         d.pushBack(i % 10);
//         ASSERT_EQ(d.size(), 10);
//     }
// }
// 
// TEST(TestDeque, backwardInchworm){
//     Deque<int> d;
//     for (int i = 0; i < 10; i++){
//         d.pushFront(i);
//     }
//     ASSERT_EQ(d.size(), 10);
//     for (int i = 0; i < 1e5; i++){
//         ASSERT_EQ(d.popBack(), i % 10);
//         ASSERT_EQ(d.peekBack(), (i + 11) % 10);
//         ASSERT_EQ(d.peekFront(), (i + 9) % 10);
//         d.pushFront(i % 10);
//         ASSERT_EQ(d.size(), 10);
//     }
// }

int main(int argc, char **argv) {
    Deque<int> d;
    d.pushBack(4);
    d.length++;
//     d.length++;
//     ::testing::InitGoogleTest(&argc, argv);
//     return RUN_ALL_TESTS();
}
