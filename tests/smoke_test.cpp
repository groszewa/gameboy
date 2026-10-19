// Placeholder: proves the doctest harness builds, links, and runs.
// Replace with per-component tests as you implement (and Blargg ROMs
// for CPU verification).

#include <doctest/doctest.h>

#include "cpu.h"
#include "bus.h"
#include "cartridge.h"
#include "ppu.h"

TEST_CASE("smoke: harness works") {
    CHECK(1 + 1 == 2);
}

TEST_CASE("smoke: components construct") {
    Bus bus;
    Cpu cpu(bus);
    Cartridge cartridge;
    Ppu ppu;
    cpu.reset();
    ppu.reset();
    CHECK(bus.read(0x0000) == 0xFF); // placeholder: unmapped reads as 0xFF
    (void)cartridge;                // nothing to assert yet
}
