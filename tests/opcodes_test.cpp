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

static void set_operand(Cpu& cpu, [[maybe_unused]] Bus& bus, int idx, uint8_t val) {
    switch(idx) {
        case 0:
            cpu.b = val;
            break;
        case 1:
            cpu.c = val;
            break;
        case 2:
            cpu.d = val;
            break;
        case 3:
            cpu.e = val;
            break;
        case 4:
            cpu.h = val;
            break;
        case 5:
            cpu.l = val;
            break;
    }
}

TEST_CASE("0xc6: add a, imm8") {
    Bus bus;
    Cpu cpu(bus);
    bus.write(0x0000, 0xc6);

    SUBCASE("no flags") {
        bus.write(0x0001, 0x05);
        cpu.reset();
        CHECK(cpu.step() == 8);
        CHECK(cpu.pc == 2);
        CHECK(cpu.a == 0x05);
        CHECK_MESSAGE(cpu.f == 0x00, "f=" << hex8(cpu.f) << " expected=0x00");
    }
    SUBCASE("half carry") {
        bus.write(0x0001, 0x01);
        cpu.reset();
        cpu.a = 0x0f;
        CHECK(cpu.step() == 8);
        CHECK(cpu.pc == 2);
        CHECK(cpu.a == 0x10);
        CHECK_MESSAGE(cpu.f == Cpu::FLAG_H, "f=" << hex8(cpu.f) << " expected=" << hex8(Cpu::FLAG_H));
    }
    SUBCASE("wraps to zero with carry") {
        bus.write(0x0001, 0x01);
        cpu.reset();
        cpu.a = 0xff;
        CHECK(cpu.step() == 8);
        CHECK(cpu.pc == 2);
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
                int cycles = cpu.step();
                if ((cpu.f & Cpu::FLAG_N) or (cycles!=8)) ++violations;
            }
        }
        CHECK(violations == 0);
    }
}

TEST_CASE("0x80-0x87: add a, reg") {
    Bus bus;
    Cpu cpu(bus);
    uint8_t opcode_base = 0x80;

    // Register field values, in opcode order
    struct Case { const char* name; int idx; };
    const Case cases[] = {{"b", 0}, {"c", 1}, {"d", 2}, {"e", 3}, {"h", 4}, {"l", 5}};
    for(const auto& tc : cases) {
        bus.write(0x0000, static_cast<uint8_t>(opcode_base + tc.idx)); // opcode = base + reg
        SUBCASE(tc.name) {
            SUBCASE("no flags") {
                cpu.reset();
                cpu.a = 0x05;
                set_operand(cpu, bus, tc.idx, 0x01);
                CHECK(cpu.step() == 4);
                CHECK(cpu.pc == 1);
                CHECK(cpu.a == 0x06);
                CHECK_MESSAGE(cpu.f == 0x00, "f=" << hex8(cpu.f) << " expected=0x00");
            }
            SUBCASE("half carry") {
                cpu.reset();
                cpu.a = 0x0f;
                set_operand(cpu, bus, tc.idx, 0x01);
                CHECK(cpu.step() == 4);
                CHECK(cpu.pc == 1);
                CHECK(cpu.a == 0x10);
                CHECK_MESSAGE(cpu.f == Cpu::FLAG_H, "f=" << hex8(cpu.f) << " expected=" << hex8(Cpu::FLAG_H));
            }
            SUBCASE("wraps to zero with carry") {
                cpu.reset();
                cpu.a = 0xff;
                set_operand(cpu, bus, tc.idx, 0x01);
                CHECK(cpu.step() == 4);
                CHECK(cpu.pc == 1);
                CHECK(cpu.a == 0x00);
                CHECK_MESSAGE(cpu.f == (Cpu::FLAG_Z | Cpu::FLAG_H | Cpu::FLAG_C),
                        "f=" << hex8(cpu.f) << " expected=" << hex8(static_cast<uint8_t>(Cpu::FLAG_Z | Cpu::FLAG_H | Cpu::FLAG_C)));
            }
            SUBCASE("N always clear") {
                // ADD must never set the subtract flag, for any of the 65536 input pairs.
                int violations = 0;
                for (int a = 0; a < 256; ++a) {
                    for (int i = 0; i < 256; ++i) {
                        cpu.reset();
                        set_operand(cpu, bus, tc.idx, i);
                        cpu.a = static_cast<uint8_t>(a);
                        int cycles = cpu.step();
                        if ((cpu.f & Cpu::FLAG_N) or (cycles!=4)) ++violations;
                    }
                }
                CHECK(violations == 0);
            }
    }
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
        CHECK(cpu.step() == 8);
        CHECK(cpu.pc == 2);
        CHECK(cpu.a == 0x00);
        CHECK_MESSAGE(cpu.f == Cpu::FLAG_Z, "f=" << hex8(cpu.f) << " expected=" << hex8(Cpu::FLAG_Z));
    }
    SUBCASE("identity with zero operand") {
        bus.write(0x0001, 0x00);
        cpu.reset();
        cpu.a = 0x42;
        CHECK(cpu.step() == 8);
        CHECK(cpu.pc == 2);
        CHECK(cpu.a == 0x42);
        CHECK_MESSAGE(cpu.f == 0x00, "f=" << hex8(cpu.f) << " expected=0x00");
    }
    SUBCASE("flips all bits") {
        bus.write(0x0001, 0xf0);
        cpu.reset();
        cpu.a = 0x0f;
        CHECK(cpu.step() == 8);
        CHECK(cpu.pc == 2);
        CHECK(cpu.a == 0xff);
        CHECK_MESSAGE(cpu.f == 0x00, "f=" << hex8(cpu.f) << " expected=0x00");
    }
    SUBCASE("zero stays zero") {
        bus.write(0x0001, 0x00);
        cpu.reset();
        cpu.a = 0x00;
        CHECK(cpu.step() == 8);
        CHECK(cpu.pc == 2);
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
                int cycles = cpu.step();
                if ((cpu.f & (Cpu::FLAG_N | Cpu::FLAG_H | Cpu::FLAG_C) or (cycles!=8))) {
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
