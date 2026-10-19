#pragma once

#include <cstdint>

// Picture Processing Unit: renders 160x144 frames from tile data.
// Timing-sensitive; get the CPU passing Blargg's cpu_instrs before this.
class Ppu {
public:
    void reset();

    // Advances the PPU by the given number of CPU cycles. (TODO)
    void step(uint32_t cycles);

private:
    // TODO: mode state (OAM search / transfer / HBlank / VBlank),
    // scanline counter, framebuffer.
};
