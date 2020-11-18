all:
	cmake -B build .
	sh -c "cd build && make"

clean:
	rm -rf ./build
