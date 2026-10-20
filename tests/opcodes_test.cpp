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
    SUBCASE("N always clear") {
        // ADD must never set the subtract flag, for any of the 65536 input pairs.
        int violations = 0;
        for (int a = 0; a < 256; ++a) {
            for (int imm = 0; imm < 256; ++imm) {
                bus.write(0x0001, static_cast<uint8_t>(imm));
                cpu.reset();
                cpu.a = static_cast<uint8_t>(a);
                cpu.step();
                if (cpu.f & Cpu::FLAG_N) ++violations;
            }
        }
        CHECK(violations == 0);
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
    SUBCASE("N/H/C always clear") {
        // XOR must never set N, H, or C, for any of the 65536 input pairs.
        int violations = 0;
        int first_a = -1, first_imm = -1;
        for (int a = 0; a < 256; ++a) {
            for (int imm = 0; imm < 256; ++imm) {
                bus.write(0x0001, static_cast<uint8_t>(imm));
                cpu.reset();
                cpu.a = static_cast<uint8_t>(a);
                cpu.step();
                if (cpu.f & (Cpu::FLAG_N | Cpu::FLAG_H | Cpu::FLAG_C)) {
                    if (violations == 0) { first_a = a; first_imm = imm; }
                    ++violations;
                }
            }
        }
        CHECK_MESSAGE(violations == 0,
                      violations << " violating pairs; first at a=" << hex8(static_cast<uint8_t>(first_a))
                                 << " imm=" << hex8(static_cast<uint8_t>(first_imm)));
    }
}
