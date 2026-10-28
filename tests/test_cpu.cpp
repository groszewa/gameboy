// CPU-level tests: register and flag behavior not tied to a single opcode.
// (Opcode execution tests live in opcodes_test.cpp.)

#include <doctest/doctest.h>

#include "cpu.h"
#include "bus.h"

TEST_CASE("flag getters and setters") {
    Bus bus;
    Cpu cpu(bus);
    cpu.reset();

    SUBCASE("setters accumulate") {
        uint8_t expected_flags = 0;
        cpu.set_z_flag(true);
        expected_flags |= Cpu::FLAG_Z;
        CHECK(cpu.f == expected_flags);
        cpu.set_n_flag(true);
        expected_flags |= Cpu::FLAG_N;
        CHECK(cpu.f == expected_flags);
        cpu.set_h_flag(true);
        expected_flags |= Cpu::FLAG_H;
        CHECK(cpu.f == expected_flags);
        cpu.set_c_flag(true);
        expected_flags |= Cpu::FLAG_C;
        CHECK(cpu.f == expected_flags);
    }
    SUBCASE("clearing flags leaves others set") {
        uint8_t expected_flags = Cpu::FLAG_Z | Cpu::FLAG_N | Cpu::FLAG_H | Cpu::FLAG_C;
        cpu.f = expected_flags;
        cpu.set_h_flag(false);
        expected_flags &= ~Cpu::FLAG_H;
        CHECK(cpu.f == expected_flags);
        cpu.set_c_flag(false);
        expected_flags &= ~Cpu::FLAG_C;
        CHECK(cpu.f == expected_flags);
    }
    SUBCASE("getters agree with raw byte for every f value") {
        for(int v = 0; v <= 0xff; v++) {
            cpu.f = static_cast<int>(v);
            CHECK(cpu.get_z_flag() == ((v & Cpu::FLAG_Z) != 0));
            CHECK(cpu.get_n_flag() == ((v & Cpu::FLAG_N) != 0));
            CHECK(cpu.get_h_flag() == ((v & Cpu::FLAG_H) != 0));
            CHECK(cpu.get_c_flag() == ((v & Cpu::FLAG_C) != 0));
        }
    }
    SUBCASE("setters never touch low nibble") {
        uint8_t expected_flags = 0xff;
        cpu.f = expected_flags;
        cpu.set_z_flag(false);
        expected_flags &= ~Cpu::FLAG_Z;
        CHECK(cpu.f == expected_flags);
        cpu.set_z_flag(true);
        expected_flags |= Cpu::FLAG_Z;
        CHECK(cpu.f == expected_flags);
    }
}
