#include "cpu.h"
#include <sstream>
#include "logging.h"

Cpu::Cpu(Bus& bus_) : bus(bus_){}

void Cpu::reset() {
    this->a = 0;
    this->b = 0;
    this->c = 0;
    this->d = 0;
    this->e = 0;
    this->h = 0;
    this->l = 0;
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
    auto r = alu_add(this->a, rhs, false);
    this->a = r.value;
    this->f = r.flags;
}

void Cpu::xor8(uint8_t rhs) {
    this->a ^= rhs;
    this->f = (this->a==0) ? FLAG_Z : 0;
}

int Cpu::step() {
    uint8_t opcode = fetch8();
    int cycles = 0; // Referece: https://gbdev.io/gb-opcodes/optables/
    //GB_LOG("Current opcode is 0x%x\n", opcode);
    switch(opcode) {
        case 0x80:
            add8(this->b);
            cycles = 4;
            break;
        case 0x81:
            add8(this->c);
            cycles = 4;
            break;
        case 0x82:
            add8(this->d);
            cycles = 4;
            break;
        case 0x83:
            add8(this->e);
            cycles = 4;
            break;
        case 0x84:
            add8(this->h);
            cycles = 4;
            break;
        case 0x85:
            add8(this->l);
            cycles = 4;
            break;
        case 0xc6:
            add8(fetch8());
            cycles = 8;
            break;
        case 0xee:
            xor8(fetch8());
            cycles = 8;
            break;
        default:
            std::ostringstream msg;
        
            msg << "Unknown opcode 0x" << std::hex << static_cast<int>(opcode);
            throw std::runtime_error(msg.str());
    }
    return cycles;
}
