#pragma once

#include <cstdint>
#include <array>

#define KB_1 1024

// Memory bus: decodes the 16-bit address space into cartridge ROM/RAM,
// VRAM, WRAM, I/O registers, etc. See the Pan Docs memory map.
class Bus {
public:
    Bus();
    uint8_t read(uint16_t addr);
    void write(uint16_t addr, uint8_t value);

private:
    // TODO: address decoding; cartridge, RAM regions, I/O registers.
    std::array<uint8_t, 64*KB_1> mem;
};
