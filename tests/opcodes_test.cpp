// Placeholder: proves the doctest harness builds, links, and runs.
// Replace with per-component tests as you implement (and Blargg ROMs
// for CPU verification).

#include <doctest/doctest.h>

#include <cstdio>
#include <string>

#include "cpu.h"
#include "bus.h"

// Format a byte as hex (e.g. 0xa0) so flag-failure messages are readable.
static std::string hex8(uint8_t v) {
    char buf[8];
    std::snprintf(buf, sizeof buf, "0x%02x", v);
    return buf;
}

TEST_CASE("0xc6: add a, imm8") {
    Bus bus;
    Cpu cpu(bus);
    bus.write(0x0000, 0xc6);

    SUBCASE("no flags") {
        bus.write(0x0001, 0x05);
        cpu.reset();
        cpu.step();
        CHECK(cpu.a == 0x05);
        CHECK_MESSAGE(cpu.f == 0x00, "f=" << hex8(cpu.f) << " expected=0x00");
    }
    SUBCASE("half carry") {
        bus.write(0x0001, 0x01);
        cpu.reset();
        cpu.a = 0x0f;
        cpu.step();
        CHECK(cpu.a == 0x10);
        CHECK_MESSAGE(cpu.f == Cpu::FLAG_H, "f=" << hex8(cpu.f) << " expected=" << hex8(Cpu::FLAG_H));
    }
    SUBCASE("wraps to zero with carry") {
        bus.write(0x0001, 0x01);
        cpu.reset();
        cpu.a = 0xff;
        cpu.step();
        CHECK(cpu.a == 0x00);
        CHECK_MESSAGE(cpu.f == (Cpu::FLAG_Z | Cpu::FLAG_H | Cpu::FLAG_C),
                      "f=" << hex8(cpu.f) << " expected=" << hex8(static_cast<uint8_t>(Cpu::FLAG_Z | Cpu::FLAG_H | Cpu::FLAG_C)));
    }
}

TEST_CASE("0xee: xor a, imm8") {
    Bus bus;
    Cpu cpu(bus);
    bus.write(0x0000, 0xee);

    SUBCASE("xors to zero") {
        bus.write(0x0001, 0xff);
        cpu.reset();
        cpu.a = 0xff;
        cpu.step();
        CHECK(cpu.a == 0x00);
        CHECK_MESSAGE(cpu.f == Cpu::FLAG_Z, "f=" << hex8(cpu.f) << " expected=" << hex8(Cpu::FLAG_Z));
    }
    SUBCASE("identity with zero operand") {
        bus.write(0x0001, 0x00);
        cpu.reset();
        cpu.a = 0x42;
        cpu.step();
        CHECK(cpu.a == 0x42);
        CHECK_MESSAGE(cpu.f == 0x00, "f=" << hex8(cpu.f) << " expected=0x00");
    }
    SUBCASE("flips all bits") {
        bus.write(0x0001, 0xf0);
        cpu.reset();
        cpu.a = 0x0f;
        cpu.step();
        CHECK(cpu.a == 0xff);
        CHECK_MESSAGE(cpu.f == 0x00, "f=" << hex8(cpu.f) << " expected=0x00");
    }
    SUBCASE("zero stays zero") {
        bus.write(0x0001, 0x00);
        cpu.reset();
        cpu.a = 0x00;
        cpu.step();
        CHECK(cpu.a == 0x00);
        CHECK_MESSAGE(cpu.f == Cpu::FLAG_Z, "f=" << hex8(cpu.f) << " expected=" << hex8(Cpu::FLAG_Z));
    }
}
