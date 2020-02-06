all: CMakeLists.txt build
	cd build && cmake .. && make

build:
	mkdir build

.PHONY: clean run debug

run: all
	cd build
	PYTHONPATH=$${PYTHONPATH}:$$(pwd)/build/lib python3 -i tgimbox.py

debug: all
	cd build
	PYTHONPATH=$${PYTHONPATH}:$$(pwd)/build/lib python3 -i debug.py

clean:
	rm -fr build
