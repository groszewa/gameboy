#include "cpu.h"
#include <sstream>
#include "logging.h"

Cpu::Cpu(Bus& bus_) : bus(bus_){}

void Cpu::reset() {
    this->a = 0;
    this->f = 0;
    this->pc = 0;
}

Cpu::AluResult Cpu::alu_add(uint8_t lhs, uint8_t rhs, bool carry_in) {
    uint16_t sum = static_cast<uint16_t>(lhs) + rhs + (carry_in ? 1 : 0);
    uint8_t flags = 0;
    if ((sum & 0xff) == 0) {
        flags  |= FLAG_Z;
    }
    if (((lhs & 0xF) + (rhs & 0xF) + (carry_in ? 1 : 0)) > 0xF) {
        flags |= FLAG_H;
    }
    if (sum > 0xff) {
        flags |= FLAG_C;
    }
    return {static_cast<uint8_t>(sum), flags};
}

void Cpu::add8(uint8_t rhs) {
    auto r = alu_add(a, rhs, false);
    a = r.value;
    f = r.flags;
}

void Cpu::xor8(uint8_t rhs) {
    a ^= rhs;
    f = (a==0) ? FLAG_Z : 0;
}

void Cpu::step() {
    uint8_t opcode = bus.read(pc++);
    //GB_LOG("Current opcode is 0x%x\n", opcode);
    switch(opcode) {
        case 0xc6:
            add8(fetch8());
            break;
        case 0xee:
            xor8(fetch8());
            break;
        default:
            std::ostringstream msg;
            msg << "Unknown opcode 0x" << std::hex << static_cast<int>(opcode);
            throw std::runtime_error(msg.str());
    }
}
