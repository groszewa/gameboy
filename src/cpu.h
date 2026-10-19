#pragma once

#include <cstdint>
#include "bus.h"

// Sharp LR35902 CPU core.
// Reference: Pan Docs (https://gbdev.io/pandocs/) — CPU registers & instruction set.
class Cpu {
public:
    explicit Cpu(Bus& bus);
    void reset();

    // Executes a single instruction. (Cycle counting comes later.)
    void step();

    uint8_t a, f;
    uint16_t pc;

    // Flag bits (upper nibble of F)
    static constexpr uint8_t FLAG_Z = 0x80;
    static constexpr uint8_t FLAG_N = 0x40;
    static constexpr uint8_t FLAG_H = 0x20;
    static constexpr uint8_t FLAG_C = 0x10;

private:
    Bus& bus;
};
