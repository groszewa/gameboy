// Placeholder: proves the doctest harness builds, links, and runs.
// Replace with per-component tests as you implement (and Blargg ROMs
// for CPU verification).

#include <doctest/doctest.h>

#include "cpu.h"
#include "bus.h"

TEST_CASE("0xc6: add a, imm8") {
    Bus bus;
    Cpu cpu(bus);
    bus.write(0x0000, 0xc6);

    SUBCASE("no flags") {
        bus.write(0x0001, 0x05);
        cpu.reset();
        cpu.step();
        CHECK(cpu.a == 0x05);
    }
    SUBCASE("half carry") {
        bus.write(0x0001, 0x01);
        cpu.reset();
        cpu.a = 0x0f;
        cpu.step();
        CHECK(cpu.a == 0x10);
        CHECK(cpu.f == Cpu::FLAG_H);
    }
    SUBCASE("wraps to zero with carry") {
        bus.write(0x0001, 0x01);
        cpu.reset();
        cpu.a = 0xff;
        cpu.step();
        CHECK(cpu.a == 0x00);
        CHECK(cpu.f == (Cpu::FLAG_Z | Cpu::FLAG_H | Cpu::FLAG_C));
    }
}
