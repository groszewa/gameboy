# gameboy

Game Boy emulator (Sharp LR35902) that plays real ROM files.

Scaffolding only: build system, test harness, and file layout. The
emulation itself is intentionally unimplemented.

## Layout

- `src/` — the emulator library (`gameboy` static lib):
  - `cpu.h` — LR35902 core (fetch/decode/execute)
  - `bus.h` — 16-bit address decoding (ROM, VRAM, WRAM, I/O)
  - `cartridge.h` — ROM loading + MBC bank switching
  - `ppu.h` — picture processing unit (160x144)
- `tests/` — doctest unit tests, run via CTest.
- `tools/` — future CLIs (disassembler, debugger).

## References

- Pan Docs (https://gbdev.io/pandocs/) — the hardware bible
- Blargg's test ROMs — CPU verification suite (`cpu_instrs.gb` first)

## Suggested bring-up order

1. CPU: registers + `step()`; test each instruction with doctest.
2. Run Blargg's `cpu_instrs.gb` — the ground truth for the CPU.
3. Cartridge: ROM loading, then MBC1/3/5 as needed.
4. PPU: tile rendering and mode timing.
5. Joypad input + main loop.

## Prerequisites

- CMake >= 3.16
- A C++20 compiler (GCC, Clang, MSVC)
- Git + network access (first configure fetches doctest)

## Build & test

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

## Adding code

- New sources: append the `.cpp` to `src/CMakeLists.txt`.
- New tests: add a `*_test.cpp` to `tests/CMakeLists.txt`; register the
  binary with CTest via `add_test` (see `tests/CMakeLists.txt`).

## Notes

- C++ standard is set in the top-level `CMakeLists.txt`
  (`CMAKE_CXX_STANDARD`).
- Tests use [doctest](https://github.com/doctest/doctest) (v2.4.11, fetched
  with CMake FetchContent). One translation unit defines
  `DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN`; add `TEST_CASE`s freely elsewhere.
- `-Wall -Wextra -Wpedantic` are on by default.
