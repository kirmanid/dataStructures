#include <iostream>
#include <exception>
#include <queue>
#include <functional>
#include "gtest/gtest.h"

class arrStack {
private:
    int* data;
    uint position;
    uint capacity;
public:
    arrStack();
    ~arrStack();
    void push(int value); //  adds value to the top of the stack
    int pop();            //  removes and returns top value of the stack
    int peek();           //  returns top value of the stack without removing it
    bool isEmpty();       //  returns true if the stack is empty, false otherwise
    int size();           //  returns the number of items on the stack
};

struct llNode {
    int data;
    llNode* next;
};

class llStack {
private:
    llNode* top;
    uint stackSize;
public:
    llStack();
    ~llStack();
    void push(int value);
    int pop();
    int peek();
    bool isEmpty();
    int size();
};

llStack::llStack()
:stackSize{0}
{}

llStack::~llStack(){
    while (stackSize > 0){
        pop();
    }
}

void llStack::push(int value){
    llNode* newTop = new llNode;
    newTop->next = top;
    newTop->data = value;
    top = newTop;
    stackSize++;
}

int llStack::pop(){
    if (stackSize < 1){
       throw new std::logic_error("Popped from empty stack"); 
    }
    llNode tempTop = *top;
    delete top;
    stackSize--;
    top = tempTop.next;
    return tempTop.data;
}

int llStack::peek(){
    if (stackSize < 1){
       throw new std::logic_error("Peeked from empty stack"); 
    }
    return top->data;
}

bool llStack::isEmpty(){
    return stackSize == 0;
}

int llStack::size(){
    return stackSize;
}

arrStack::arrStack()
:position{0},
capacity{32}
{
    data = new int[capacity];
}

arrStack::~arrStack(){
    delete [] data;
}

void arrStack::push(int value){
    if (position == capacity){
        int* oldData = new int[capacity];
        for (uint i = 0; i < capacity; i++){
            oldData[i] = data[i];
        }
        capacity *= 2;
        data = new int[capacity];
        for (uint i = 0; i < capacity/2; i++){
            data[i] = oldData[i];
        }
    }
    data[position] = value;
    position++;
    return;
}

int arrStack::pop(){
    if (position < 1){
        throw new std::logic_error("Popped from empty stack");
    }
    position--;
    return data[position];
}

int arrStack::peek(){
    if (position < 1){
        throw new std::logic_error("Peeked from empty stack");
    }
    return data[position - 1];
}
/// pointer to data or actual data here?
bool arrStack::isEmpty(){
    return position==0;
}

int arrStack::size(){
    return position;
}

TEST(TestStack, NewStackIsEmpty) {
    llStack s;
    ASSERT_TRUE(s.isEmpty());
    ASSERT_EQ(s.size(), 0);
}

TEST(TestStack, OneElementStackHasSizeOne) {
    llStack s;
    s.push(93);
    ASSERT_EQ(s.size(), 1);
    ASSERT_FALSE(s.isEmpty());
}

TEST(TestStack, PopNPeekFromEmptyStack){
    llStack s;
    ASSERT_THROW(s.peek(), std::logic_error*);
    ASSERT_THROW(s.pop(), std::logic_error*);
}

TEST(TestStack, PushNPeek) {
    llStack s;
    s.push(34);
    ASSERT_EQ(s.peek(), 34);
}

TEST(TestStack, PushNPop) {
    llStack s;
    s.push(36);
    ASSERT_EQ(s.pop(), 36);
}

TEST(TestStack, PushNPop2){
    llStack s;
    s.push(287);
    s.push(91);
    ASSERT_EQ(s.size(), 2);
    ASSERT_EQ(s.peek(), 91);
    ASSERT_EQ(s.pop(), 91);
    ASSERT_EQ(s.pop(), 287);
}

TEST(TestStack, PushNPopSequence){
    uint i = 1;
    llStack s;
    for (; i <= 1e6; i++){
        s.push(i);
    }
    i--;
    for (; i >= 1; i--){
        ASSERT_EQ(s.pop(), i);
    }
    
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
