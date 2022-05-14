CXX = c++
CXXFLAGS = -Wall -Wextra -g -std=c++11

SRC = bst.cpp            \


EXE = $(SRC:.cpp=.x)

.SUFFIXES:
SUFFIXES =

.SUFFIXES: .cpp .x

all: $(EXE)

.PHONY: all

%.x: %.cpp 
	$(CXX) $< -o $@ $(CXXFLAGS)
