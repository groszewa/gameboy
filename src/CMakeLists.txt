add_library(gameboy STATIC
    cpu.cpp
    bus.cpp
    cartridge.cpp
    ppu.cpp
)

# Public so tests (and later tools/) can #include the headers.
target_include_directories(gameboy PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
