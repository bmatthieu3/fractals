
CXX=g++
CXXFLAGS=
LDFLAGS=-lglfw3 -lGL -lX11 -lpthread -lXrandr -lXi -ldl
INC=-Iinclude/
EXEC=asia-engine
SRC= $(wildcard src/*.cpp)
OBJ= $(SRC:.cpp=.o)

all: $(EXEC)
	./$<

asia-engine: $(OBJ)
	$(CXX) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) -o $@ -c $< $(INC) $(CXXFLAGS)

.PHONY: clean mrproper

clean:
	rm -rf src/*.o

mrproper: clean
	rm -rf $(EXEC)