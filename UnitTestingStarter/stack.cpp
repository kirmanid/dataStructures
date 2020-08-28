#include <iostream>
#include <queue>
#include <functional>
#include "gtest/gtest.h"

/*
void push(int value); //  adds value to the top of the stack
int pop();            //  removes and returns top value of the stack
int peek();           //  returns top value of the stack without removing it
bool isEmpty();       //  returns true if the stack is empty, false otherwise
int size();           //  returns the number of items on the stack*/

class myVector {

private:

public:

    myVector();

    void push_back(double v);

    double get(int index);

    int size();

};


myVector::myVector()

{

}


void myVector::push_back(double v)

{

}


double myVector::get(int index)

{

    return 324234235;

}


int myVector::size()

{

    return 324;

}


TEST(TestVector, NewVectorIsEmpty) {

    myVector v;

//    ASSERT_TRUE(v.size() == 0);

    ASSERT_EQ(v.size(), 0);

}


TEST(TestVector, SizeIsOneAfterOnePushBack) {

    myVector v;

    v.push_back(37);

    ASSERT_EQ(v.size(), 1);

}


TEST(TestVector, CanRetrievePushedValue) {

    myVector v;

    v.push_back(37);

    ASSERT_EQ(v.get(0), 37);

}


int main(int argc, char **argv)  {

    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();

}
