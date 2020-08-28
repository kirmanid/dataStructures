#include <iostream>
#include <queue>
#include <functional>

#include "gtest/gtest.h"

using namespace std;

/// Stack class specs
///
/// Name of the class:  Stack
///
/// Public methods:
/// void push(int value)   adds value to the top of the stack
/// int pop()  removes and returns top value of the stack
/// int peek()  returns top value of the stack without removing it
/// bool isEmpty()  returns true if the stack is empty, false otherwise
/// int size() returns the number of items on the stack
///
/// push, isEmpty, and size should work in all cases
/// pop and peek will only be called on non-empty stacks


//TEST(TestStack, EmptyStackIsEmpty) {
//    Stack s;
//    ASSERT_TRUE(s.isEmpty());
//}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}


