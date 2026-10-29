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

static void set_operand(Cpu& cpu, Bus& bus, int idx, uint8_t val) {
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
        case 6:
            {
                cpu.h = 0xc0;
                cpu.l = 0x00;
                bus.write((cpu.h << 8) | cpu.l, val);
            }
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
        cpu.f = (Cpu::FLAG_Z | Cpu::FLAG_H | Cpu::FLAG_C | Cpu::FLAG_N); // "dirty" flags not propogated
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

static void check_add_family(uint8_t opcode_base, bool use_carry) {
    Bus bus;
    Cpu cpu(bus);

    // Register field values, in opcode order
    struct Case { const char* name; int idx; };
    const Case cases[] = {{"b", 0}, {"c", 1}, {"d", 2}, {"e", 3}, {"h", 4}, {"l", 5}, {"[hl]", 6}};
    for(const auto& tc : cases) {
        int expected_cycles = (tc.idx==6) ? 8 : 4;
        bus.write(0x0000, static_cast<uint8_t>(opcode_base + tc.idx)); // opcode = base + reg
        SUBCASE(tc.name) {
            SUBCASE("no flags") {
                for (int cin = 0; cin <= (use_carry ? 1 : 0); ++cin) {
                    int eff_cin = use_carry ? cin : 0;
                    cpu.reset();
                    cpu.a = 0x05;
                    set_operand(cpu, bus, tc.idx, 0x01);
                    cpu.f = use_carry ? (cin ? 0xF0 : 0xE0) : 0xF0;
                    CHECK(cpu.step() == expected_cycles);
                    CHECK(cpu.pc == 1);
                    CHECK(cpu.a == 0x06 + eff_cin);
                    CHECK_MESSAGE(cpu.f == 0x00, "f=" << hex8(cpu.f) << " expected=0x00");
                }
            }
            SUBCASE("half carry") {
                for (int cin = 0; cin <= (use_carry ? 1 : 0); ++cin) {
                    int eff_cin = use_carry ? cin : 0;
                    cpu.reset();
                    cpu.a = 0x0f;
                    set_operand(cpu, bus, tc.idx, 0x01);
                    cpu.f = use_carry ? (cin ? 0xF0 : 0xE0) : 0xF0;
                    CHECK(cpu.step() == expected_cycles);
                    CHECK(cpu.pc == 1);
                    CHECK(cpu.a == 0x10 + eff_cin);
                    CHECK_MESSAGE(cpu.f == Cpu::FLAG_H, "f=" << hex8(cpu.f) << " expected=" << hex8(Cpu::FLAG_H));
                }
            }
            SUBCASE("wraps to zero with carry") {
                for (int cin = 0; cin <= (use_carry ? 1 : 0); ++cin) {
                    int eff_cin = use_carry ? cin : 0;
                    cpu.reset();
                    cpu.a = 0xff;
                    set_operand(cpu, bus, tc.idx, 0x01);
                    cpu.f = use_carry ? (cin ? 0xF0 : 0xE0) : 0xF0;
                    int sum = 0xff + 0x01 + eff_cin;
                    uint8_t exp_a = static_cast<uint8_t>(sum);
                    uint8_t exp_f = (exp_a == 0 ? Cpu::FLAG_Z : 0) | Cpu::FLAG_H | Cpu::FLAG_C;
                    CHECK(cpu.step() == expected_cycles);
                    CHECK(cpu.pc == 1);
                    CHECK(cpu.a == exp_a);
                    CHECK_MESSAGE(cpu.f == exp_f, "f=" << hex8(cpu.f) << " expected=" << hex8(exp_f));
                }
            }
            SUBCASE("exhaustive model") {
                // Every (a, operand) pair, under every carry-in state the op
                // admits: result and full flag byte must match the model.
                int violations = 0;
                int first_a = -1, first_op = -1, first_cin = -1;
                for (int cin = 0; cin <= (use_carry ? 1 : 0); ++cin) {
                    int eff_cin = use_carry ? cin : 0;
                    uint8_t preset = use_carry ? (cin ? 0xF0 : 0xE0) : 0xF0;
                    for (int a = 0; a < 256; ++a) {
                        for (int i = 0; i < 256; ++i) {
                            cpu.reset();
                            set_operand(cpu, bus, tc.idx, static_cast<uint8_t>(i));
                            cpu.a = static_cast<uint8_t>(a);
                            cpu.f = preset;
                            int cycles = cpu.step();
                            int sum = a + i + eff_cin;
                            uint8_t exp_a = static_cast<uint8_t>(sum);
                            uint8_t exp_f = (exp_a == 0 ? Cpu::FLAG_Z : 0)
                                          | (((a & 0xF) + (i & 0xF) + eff_cin) > 0xF ? Cpu::FLAG_H : 0)
                                          | (sum > 0xFF ? Cpu::FLAG_C : 0);
                            if ((cpu.a != exp_a) or (cpu.f != exp_f) or (cycles != expected_cycles)) {
                                if (violations == 0) { first_a = a; first_op = i; first_cin = cin; }
                                ++violations;
                            }
                        }
                    }
                }
                CHECK_MESSAGE(violations == 0,
                              violations << " violating inputs; first at a=" << hex8(static_cast<uint8_t>(first_a))
                                         << " op=" << hex8(static_cast<uint8_t>(first_op))
                                         << " cin=" << first_cin);
            }
        }
    }
}

TEST_CASE("0x80-0x86: add a, reg") {
    check_add_family(0x80, false);
}

TEST_CASE("0x88-0x8e: adc a, reg") {
    check_add_family(0x88, true);
}

static void check_hl_addressing(uint8_t opcode) {
    Bus bus;
    Cpu cpu(bus);
    // 2 copies same opcode to enable b2b execution
    bus.write(0x0000, opcode);
    bus.write(0x0001, opcode);

    // Register sweep always points hl at 0xc000, so it can't catch an implementation that reads a fixed
    // or misdecoded address.  Vary HL and check that the value added follows the pointer
    SUBCASE("follow the pointer") {
        uint16_t addr1 = 0xc000;
        uint8_t  data1 = 0x10;
        bus.write(addr1, data1);
        uint16_t addr2 = 0xc100;
        uint8_t  data2 = 0x20;
        bus.write(addr2, data2);
        cpu.reset();
        cpu.h = addr1 >> 8;
        cpu.l = addr1 & 0xff;
        CHECK(cpu.step() == 8);
        CHECK(cpu.pc == 1);
        CHECK(cpu.a == data1);
        cpu.h = addr2 >> 8;
        cpu.l = addr2 & 0xff;
        CHECK(cpu.step() == 8);
        CHECK(cpu.pc == 2);
        CHECK(cpu.a == data1 + data2);
    }
    // Only l changes between steps: catches implementation that drops the low byte and always reads the
    // page base (H << 8)
    SUBCASE("full address decoded") {
        uint16_t addr1 = 0xc000;
        uint8_t  data1 = 0x10;
        bus.write(addr1, data1);
        uint16_t addr2 = 0xc001;
        uint8_t  data2 = 0x20;
        bus.write(addr2, data2);
        cpu.reset();
        cpu.h = addr1 >> 8;
        cpu.l = addr1 & 0xff;
        CHECK(cpu.step() == 8);
        CHECK(cpu.pc == 1);
        CHECK(cpu.a == data1);
        cpu.h = addr2 >> 8;
        cpu.l = addr2 & 0xff;
        CHECK(cpu.step() == 8);
        CHECK(cpu.pc == 2);
        CHECK(cpu.a == data1 + data2);
    }
    // Plain (HL) addressing never modifies the pair (unlike hl+/hl- load instructions).  Check h and l
    // directly, not just result
    SUBCASE("h and l unchanged") {
        uint16_t addr1 = 0xc000;
        uint8_t  data1 = 0x10;
        bus.write(addr1, data1);
        cpu.reset();
        cpu.h = addr1 >> 8;
        cpu.l = addr1 & 0xff;
        CHECK(cpu.step() == 8);
        CHECK(cpu.pc == 1);
        CHECK(cpu.a == data1);
        CHECK(cpu.step() == 8);
        CHECK(cpu.pc == 2);
        CHECK(cpu.a == 2*data1);
        CHECK(cpu.h == (addr1 >> 8));
        CHECK(cpu.l == (addr1 & 0xff));
    }
}

TEST_CASE("0x86: add a, [hl]") {
    check_hl_addressing(0x86);
}

TEST_CASE("0x8e: adc a, [hl]") {
    check_hl_addressing(0x8e);
}

static void check_add_a_a(uint8_t opcode, bool use_carry) {
    Bus bus;
    Cpu cpu(bus);
    bus.write(0x0000, opcode);
    bus.write(0x0001, opcode);
    bus.write(0x0002, opcode);

    SUBCASE("no flags") {
        for (int cin = 0; cin <= (use_carry ? 1 : 0); ++cin) {
            int eff_cin = use_carry ? cin : 0;
            cpu.reset();
            cpu.a = 0x01;
            cpu.f = use_carry ? (cin ? 0xF0 : 0xE0) : 0xF0;
            uint8_t expected = static_cast<uint8_t>(2 * 0x01 + eff_cin);
            CHECK(cpu.step() == 4);
            CHECK(cpu.pc == 1);
            CHECK(cpu.a == expected);
            CHECK_MESSAGE(cpu.f == 0x00, "f=" << hex8(cpu.f) << " expected=0x00");
            expected = static_cast<uint8_t>(2 * expected);
            CHECK(cpu.step() == 4);
            CHECK(cpu.pc == 2);
            CHECK(cpu.a == expected);
            CHECK_MESSAGE(cpu.f == 0x00, "f=" << hex8(cpu.f) << " expected=0x00");
            expected = static_cast<uint8_t>(2 * expected);
            CHECK(cpu.step() == 4);
            CHECK(cpu.pc == 3);
            CHECK(cpu.a == expected);
            CHECK_MESSAGE(cpu.f == 0x00, "f=" << hex8(cpu.f) << " expected=0x00");
        }
    }
    SUBCASE("0x0 sets zero flag") {
        for (int cin = 0; cin <= (use_carry ? 1 : 0); ++cin) {
            int eff_cin = use_carry ? cin : 0;
            cpu.reset();
            cpu.f = use_carry ? (cin ? 0xF0 : 0xE0) : 0xF0;
            uint8_t exp_a = static_cast<uint8_t>(eff_cin);
            uint8_t exp_f = eff_cin ? 0 : Cpu::FLAG_Z;
            CHECK(cpu.step() == 4);
            CHECK(cpu.pc == 1);
            CHECK(cpu.a == exp_a);
            CHECK_MESSAGE(cpu.f == exp_f, "f=" << hex8(cpu.f) << " expected=" << hex8(exp_f));
        }
    }
    SUBCASE("0x80 boundary") {
        for (int cin = 0; cin <= (use_carry ? 1 : 0); ++cin) {
            int eff_cin = use_carry ? cin : 0;
            cpu.reset();
            cpu.a = 0x80;
            cpu.f = use_carry ? (cin ? 0xF0 : 0xE0) : 0xF0;
            int sum = 0x80 + 0x80 + eff_cin;
            uint8_t exp_a = static_cast<uint8_t>(sum);
            uint8_t exp_f = (exp_a == 0 ? Cpu::FLAG_Z : 0) | Cpu::FLAG_C;
            CHECK(cpu.step() == 4);
            CHECK(cpu.pc == 1);
            CHECK(cpu.a == exp_a);
            CHECK_MESSAGE(cpu.f == exp_f, "f=" << hex8(cpu.f) << " expected=" << hex8(exp_f));
        }
    }
    SUBCASE("FLAG_H set when a >= 0x08") {
        for (int cin = 0; cin <= (use_carry ? 1 : 0); ++cin) {
            for(uint8_t i = 0x0; i < 0x08; i++) {
                cpu.reset();
                cpu.a = i;
                cpu.f = use_carry ? (cin ? 0xF0 : 0xE0) : 0xF0;
                cpu.step();
                CHECK((cpu.f & Cpu::FLAG_H) == 0x00);
            }
            for(uint8_t i = 0x08; i < 0x10; i++) {
                cpu.reset();
                cpu.a = i;
                cpu.f = use_carry ? (cin ? 0xF0 : 0xE0) : 0xF0;
                cpu.step();
                CHECK((cpu.f & Cpu::FLAG_H) == Cpu::FLAG_H);
            }
        }
    }
    SUBCASE("FLAG_C set when >= 0x80") {
        for (int cin = 0; cin <= (use_carry ? 1 : 0); ++cin) {
            for(uint8_t i = 0x0; i < 0x80; i++) {
                cpu.reset();
                cpu.a = i;
                cpu.f = use_carry ? (cin ? 0xF0 : 0xE0) : 0xF0;
                cpu.step();
                CHECK((cpu.f & Cpu::FLAG_C) == 0x00);
            }
            for(uint16_t i = 0x0080; i < 0x0100; i++) {
                cpu.reset();
                cpu.a = static_cast<uint8_t>(i);
                cpu.f = use_carry ? (cin ? 0xF0 : 0xE0) : 0xF0;
                cpu.step();
                CHECK((cpu.f & Cpu::FLAG_C) == Cpu::FLAG_C);
            }
        }
    }
}

TEST_CASE("0x87: add, a, a") {
    check_add_a_a(0x87, false);
}

TEST_CASE("0x8f: adc, a, a") {
    check_add_a_a(0x8f, true);
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
