#pragma once

#include <cstdint>
#include "bus.h"

// Sharp LR35902 CPU core.
// Reference: Pan Docs (https://gbdev.io/pandocs/) — CPU registers & instruction set.
class Cpu {
public:
    explicit Cpu(Bus& bus);
    void reset();

    // Executes a single instruction and returns elapsed cycles
    int step();

    uint8_t a, f;
    uint16_t pc;

    // Flag bits (upper nibble of F)
    static constexpr uint8_t FLAG_Z = 0x80;
    static constexpr uint8_t FLAG_N = 0x40;
    static constexpr uint8_t FLAG_H = 0x20;
    static constexpr uint8_t FLAG_C = 0x10;

private:
    Bus& bus;

    uint8_t fetch8() { return bus.read(pc++); };

    struct AluResult {
        uint8_t value;
        uint8_t flags;
    };
    static AluResult alu_add(uint8_t lhs, uint8_t rhs, bool carry_in);

    // Thin wrappers: apply result to a/f
    void add8(uint8_t rhs);
    void xor8(uint8_t rhs);
};
