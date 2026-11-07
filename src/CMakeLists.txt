add_library(gameboy STATIC
    microcode.cpp
    cpu.cpp
    bus.cpp
    cartridge.cpp
    ppu.cpp
)

target_compile_definitions(gameboy PRIVATE GAMEBOY_OPCODES_JSON="${CMAKE_SOURCE_DIR}/data/opcodes.json")
target_link_libraries(gameboy PRIVATE nlohmann_json::nlohmann_json)

# Public so tests (and later tools/) can #include the headers.
target_include_directories(gameboy PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
