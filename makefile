CXX=g++
CXXFLAGS=-std=c++1z
OS=$(shell uname)
ifeq ($(OS),Darwin)
	LDFLAGS=-lglfw3 -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo -lpthread -ldl
else
	LDFLAGS=-lglfw3 -lXinerama -lXxf86vm -lXcursor -lGL -lX11 -lpthread -lXrandr -lXi -ldl
endif
INC=-Iinclude/
EXEC=fractals
SRC= $(wildcard src/*.cpp)
OBJ= $(SRC:.cpp=.o)

all: $(EXEC)
	./$<

fractals: $(OBJ)
	$(CXX) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) -o $@ -c $< $(INC) $(CXXFLAGS)

.PHONY: clean mrproper

clean:
	rm -rf src/*.o

mrproper: clean
	rm -rf $(EXEC)