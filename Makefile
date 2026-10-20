# Convenience shortcuts. The real build is CMake; this just saves typing.
#   make        -> configure + build
#   make test   -> build + run tests
#   make clean  -> remove the build directory

BUILD_DIR  := build
BUILD_TYPE := Debug
SANTIZE    := OF

.PHONY: all build test clean

all: build

build:
	cmake -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)
	cmake --build $(BUILD_DIR) --parallel

test: build
	ctest --test-dir $(BUILD_DIR) --output-on-failure

clean:
	rm -rf $(BUILD_DIR)
