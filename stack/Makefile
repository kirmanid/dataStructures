m : stack.o gtest-all.o
	g++ -o m -pthread stack.o gtest-all.o

stack.o : stack.cpp
	g++ -c stack.cpp -I./include

gtest-all.o : ./src/gtest-all.cc
	g++ -c "src/gtest-all.cc" -I./include -I./
