# Указываем компилятор и базовые флаги
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2

all: comdiff

comdiff: comdiff.o realis.o
	$(CXX) $(CXXFLAGS) -o comdiff comdiff.o realis.o
comdiff.o: comdiff.cpp head.hpp
	$(CXX) $(CXXFLAGS) -c comdiff.cpp
realis.o: realis.cpp head.hpp
	$(CXX) $(CXXFLAGS) -c realis.cpp
test: test.o realis.o
	$(CXX) $(CXXFLAGS) -o test test.o realis.o
	./test
test.o: test.cpp head.hpp
	$(CXX) $(CXXFLAGS) -c test.cpp
clean:
	rm -f *.o comdiff test
