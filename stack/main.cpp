#include <iostream>
#include <queue>
#include <functional>
#include "gtest/gtest.h"

class myStack {
public:
    void push(int value); //  adds value to the top of the stack
    int pop();            //  removes and returns top value of the stack
    int peek();           //  returns top value of the stack without removing it
    bool isEmpty();       //  returns true if the stack is empty, false otherwise
    int size();           //  returns the number of items on the stack
};

void myStack::push(int value){
    return;
}

int myStack::pop(){
    return 1022;
}

int myStack::peek(){
    return 1021;
}

bool myStack::isEmpty(){
    return true;
}

int myStack::size(){
    return 486;
}

TEST(TestStack, NewStackIsEmpty) {
    myStack s;
    ASSERT_EQ(s.size(), 0);
}

TEST(TestStack, OneElementStackHasSizeOne) {
    myStack s;
    s.push(93);
    ASSERT_EQ(s.size(), 1);
}

TEST(TestStack, PushNPeek) {
    myStack s;
    s.push(34);
    ASSERT_EQ(s.peek(), 34);
}

TEST(TestStack, PushNPop) {
    myStack s;
    s.push(36);
    ASSERT_EQ(s.pop(), 36);
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
