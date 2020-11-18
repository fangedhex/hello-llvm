CXX=clang++
CXX_FLAGS=$(shell llvm-config --cxxflags) -Wall
LD_FLAGS=$(shell llvm-config --libs)

all:
	$(CXX) -o hello $(CXX_FLAGS) $(LD_FLAGS) src/main.cpp

clean:
	rm -rf ./build
