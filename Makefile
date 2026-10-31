# Convenience shortcuts. The real build is CMake; this just saves typing.
#   make                  -> configure + build
#   make test             -> build + run tests
#   make SANITIZE=ON test -> build + run tests under AddressSanitizer + UBSan
#   make coverage         -> coverage build + gcovr HTML report in coverage/
#   make clean            -> remove the build directories and coverage output

BUILD_DIR              := build
BUILD_TYPE             := Debug
COVERAGE_BUILD_DIR     := build-cov
COVERAGE_ARTIFACTS_DIR := coverage
SANITIZE               := OFF
GCOV_EXECUTABLE        = $(shell sh scripts/gcov-executable.sh)

.PHONY: all build test coverage clean

all: build

build:
	cmake -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) -DGAMEBOY_SANITIZE=$(SANITIZE)
	cmake --build $(BUILD_DIR) --parallel

test: build
	ctest --test-dir $(BUILD_DIR) --output-on-failure

coverage:
	cmake -B $(COVERAGE_BUILD_DIR) -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) -DGAMEBOY_COVERAGE=ON
	cmake --build $(COVERAGE_BUILD_DIR) --parallel
	./$(COVERAGE_BUILD_DIR)/tests/gb_tests
	mkdir -p $(COVERAGE_ARTIFACTS_DIR)
	gcovr -r . --object-directory $(COVERAGE_BUILD_DIR) --exclude '.*_deps.*' --exclude 'tests/.*'  --gcov-executable "$(GCOV_EXECUTABLE)" --html --html-details -o $(COVERAGE_ARTIFACTS_DIR)/index.html

clean:
	rm -rf $(BUILD_DIR) $(COVERAGE_BUILD_DIR) $(COVERAGE_ARTIFACTS_DIR)
