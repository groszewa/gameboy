#pragma once

#include <cstdint>

// ROM cartridge: loads a .gb file, handles MBC bank switching.
// Start with header/MBC detection; implement mappers as needed (MBC1/3/5).
class Cartridge {
public:
    // TODO: bool load(const char* path);

private:
    // TODO: ROM/RAM banks, MBC type, banking registers.
};
