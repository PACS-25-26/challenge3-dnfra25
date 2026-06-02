CXX = mpic++
CXXFLAGS = -O3 -std=c++20 -fopenmp

TARGET = solver

SRC = main.cpp grid.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)
