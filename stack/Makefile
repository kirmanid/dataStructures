m : main.o gtest-all.o
	g++ -o m -pthread main.o gtest-all.o

main.o : main.cpp
	g++ -c main.cpp -I./include

gtest-all.o : ./src/gtest-all.cc
	g++ -c "src/gtest-all.cc" -I./include -I./

clean : 
	rm *.o m
