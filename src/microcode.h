#pragma once

#include <array>
#include <cstdint>
#include <string>

struct OpcodeInfo {
    std::string mnemonic;
    uint8_t cycles;
    uint8_t cycles_alt; // for conditional branches
    uint8_t bytes;
    bool illegal;
};

class Microcode {
    public:
        explicit Microcode(const std::string& path); // parse JSON, fills both arrays

        std::array<OpcodeInfo, 256> unprefixed;
        std::array<OpcodeInfo, 256> cbprefixed;
};
