# gameboy

A Game Boy (Sharp LR35902) emulator written in C++20, developed test-first
with [doctest](https://github.com/doctest/doctest). The goal is to run real
ROM files, verified against Blargg's `cpu_instrs` test ROM.

**Coverage report:** https://groszewa.github.io/gameboy/coverage/
(regenerated and republished on every push to `main`)

## Status

- **CPU** — `Cpu::step()` executes one instruction and returns its cycle
  count; an unknown opcode throws `std::runtime_error`. Implemented so far:
  the ADD row (`0x80`–`0x87`) and immediate (`0xC6`), the ADC row
  (`0x88`–`0x8F`), and XOR A,d8 (`0xEE`). Flags live in the `F` register and
  are only touched through the `get_*_flag()` / `set_*_flag()` accessors.
- **Bus** — a flat 64 KiB address space; real address decoding arrives with
  cartridge and I/O support.
- **Cartridge / PPU** — stubs for now.
- **Tests** — 11 doctest cases, 2,200+ assertions, including exhaustive
  (every input pair, exact result and exact flag byte) sweeps for the
  implemented ALU rows.

Roadmap, in order: immediate ADC (`0xCE`) · cycle table loaded from the
gb-opcodes JSON · basic ROM loading · Blargg `cpu_instrs` stood up as an
expected-fail test · the remaining opcodes · `cpu_instrs` passing.

## Prerequisites

- CMake >= 3.16
- A C++20 compiler (GCC or Clang)
- Git + network access on first configure (doctest is fetched with
  FetchContent; afterwards it is cached in the build directory)
- [gcovr](https://gcovr.com) — only for the coverage task
  (`brew install gcovr`, or `pip install gcovr`)

## Building

```sh
make            # configure + build into build/
```

or the underlying CMake, if you prefer:

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
```

CMake options (all cached per build directory):

| Option | Default | Effect |
| --- | --- | --- |
| `GAMEBOY_BUILD_TESTS` | `ON` | Build the doctest suite |
| `GAMEBOY_SANITIZE` | `OFF` | AddressSanitizer + UBSan |
| `GAMEBOY_COVERAGE` | `OFF` | gcov coverage instrumentation (`--coverage`) |

`-Wall -Wextra -Wpedantic` are always on.

## Running the tests

```sh
make test                 # build + run via CTest
make SANITIZE=ON test     # the daily driver; also what CI runs
```

The Makefile forwards `SANITIZE` to CMake on every configure, so switching
it on and off does not require wiping the build directory.

For doctest's own filtering and output, run the test binary directly:

```sh
./build/tests/gb_tests -tc="*adc*"     # run cases matching a pattern
```

Note: `ctest --output-on-failure` hides the stdout of passing tests, so
`GB_LOG` traces are only visible when running the binary directly (or with
`ctest -V`).

## Coverage

```sh
make coverage
```

This configures a separate instrumented build in `build-cov/`, runs the
tests, and writes an HTML report (gcovr) to `coverage/index.html`, scoped
to `src/` — test code and the fetched doctest sources are excluded. The
same report is published by CI on every push to `main`:
https://groszewa.github.io/gameboy/coverage/

The gcov reader differs by platform (Clang data needs `llvm-cov gcov`,
GCC data needs GNU `gcov`), so the Makefile asks
`scripts/gcov-executable.sh` to probe for the first reader that actually
runs instead of keying off the OS name.

Treat the report as an auditor, not a score: the uncovered-lines list is
the useful part — code that exists but no test has executed.

## Cleaning

```sh
make clean      # removes build/, build-cov/, and coverage/
```

## Repository layout

- `src/` — the emulator, built as the static library `gameboy`:
  - `cpu.h` / `cpu.cpp` — LR35902 core (fetch/decode/execute)
  - `bus.h` / `bus.cpp` — address space, read/write
  - `cartridge.h` / `cartridge.cpp` — ROM loading + MBC bank switching (stub)
  - `ppu.h` / `ppu.cpp` — picture processing unit, 160x144 (stub)
  - `logging.h` — `GB_LOG(...)`, debug-only logging (no-op in Release)
- `tests/` — doctest suite (`gb_tests`): `main.cpp` is the only file that
  defines `DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN`; `opcodes_test.cpp` holds
  the per-opcode cases; `test_cpu.cpp` the CPU/flag-accessor cases;
  `smoke_test.cpp` the harness smoke tests.
- `tools/` — planned CLIs (disassembler, debugger); see `tools/README.md`.
- `scripts/` — build helpers (`gcov-executable.sh`).
- `.github/workflows/ci.yml` — CI and the Pages deployment (see below).

## Adding an opcode

Development is test-driven, one opcode at a time:

1. Write the failing `TEST_CASE` in `tests/opcodes_test.cpp` first — one
   case per opcode, `SUBCASE`s on the flag boundaries, expectations
   computed (never copied from the implementation's own arithmetic), and
   the full flag byte asserted, in hex, in the failure message. Register
   rows share helpers (e.g. `check_add_family`), extended per-opcode with
   that op's flag contract.
2. Implement the `case` in `Cpu::step()`; apply flags through the
   accessors — no manual bit twiddling in opcode wrappers.
3. `make SANITIZE=ON test` must be green.

Behavior is checked against Pan Docs and the gb-opcodes data (below), not
against other emulators.

## Continuous integration

`.github/workflows/ci.yml` has two jobs:

- **build-and-test** — on pushes to `main` and on pull requests:
  `make SANITIZE=ON test` on Ubuntu.
- **coverage** — on pushes to `main` only: `make coverage`, then the
  report is deployed to GitHub Pages (see Coverage above).

Direct pushes to `main` are the norm for this project; CI is the smoke
alarm, not a merge gate.

## Contributing

This is a personal project, but the process below is the process — for
future me, or anyone who ends up with write access:

1. **Set up** — clone, then `make test` once to fetch doctest and verify
   a green baseline before changing anything.
2. **Make the change test-first** — emulator behavior changes come with a
   failing test written before the implementation (see "Adding an
   opcode"). Build-tooling changes should leave every Makefile target
   working: `build`, `test`, `coverage`, `clean`.
3. **Verify before pushing**
   - `make SANITIZE=ON test` is green.
   - No new warnings under `-Wall -Wextra -Wpedantic`.
   - If you touched dispatch or the ALU helpers, `make coverage` shows no
     newly-uncovered lines in `src/`.
4. **Commit** — short, imperative, prefixed by area: `CPU:`, `tests:`,
   `build:`, `CI:`. One logical change per commit; unrelated cleanups ride
   in their own commit, not in yours. Examples from the history:
   - `CPU: add XOR A,d8 (0xEE); tests: hex flag values in failures`
   - `tests: generalize add-family helpers for ADC`
   - `CI: publish gcovr coverage report to GitHub Pages on push`
5. **Push** — direct to `main`. CI runs the sanitizer suite; on push it
   also republishes the coverage report. A red run is fixed or reverted,
   not pushed around.

Conventions that are load-bearing (changing them needs a deliberate
decision, not a drive-by):

- Declarations in headers, definitions in `.cpp` files.
- Flags are applied individually through the `set_*_flag()` accessors;
  pure helpers compute flag conditions, wrappers apply them.
- Debug output goes through `GB_LOG` so Release builds stay silent; no
  raw `printf` in emulator code.
- New emulator sources are appended to `src/CMakeLists.txt`; new test
  files to the `gb_tests` sources in `tests/CMakeLists.txt` (and never a
  second doctest main).

## References

- [Pan Docs](https://gbdev.io/pandocs/) — the hardware bible
- [gb-opcodes JSON](https://gbdev.io/gb-opcodes/Opcodes.json) — opcode,
  operand, and cycle data (the future cycle table's source)
- [Blargg's test ROMs](https://github.com/retrio/gb-test-roms) — CPU
  verification suite (`cpu_instrs.gb` first)
