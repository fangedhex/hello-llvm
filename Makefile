CXX=clang++
FLAGS=$(shell llvm-config --cxxflags --ldflags --system-libs --libs all) -Wall

all:
	$(CXX) -o helloc $(FLAGS) src/main.cpp
	@echo "---------------------------------"
	./helloc
	@echo "---------------------------------"
	ld -dynamic-linker /lib/ld-linux-x86-64.so.2 -o hello_world -lc output.o
	@echo "---------------------------------"
	./hello_world

clean:
	rm -rf ./build
