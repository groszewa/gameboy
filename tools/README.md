# tools/

Future command-line tools live here: disassembler, debugger, ROM info, etc.

Each tool gets its own subdirectory with a `CMakeLists.txt` building an
executable linked against the `gameboy` library, e.g.:

```cmake
add_executable(gbdis gbdis.cpp)
target_link_libraries(gbdis PRIVATE gameboy)
```

A disassembler is especially handy early on: decoding opcodes by hand
while implementing the CPU gets old fast.
