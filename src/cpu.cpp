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

bool Cpu::get_z_flag() {
    return this->f & FLAG_Z;
}

void Cpu::set_z_flag(bool v) {
    this->f = (f & ~FLAG_Z) | (v ? FLAG_Z : 0);
}

bool Cpu::get_n_flag() {
    return this->f & FLAG_N;
}

void Cpu::set_n_flag(bool v) {
    this->f = (f & ~FLAG_N) | (v ? FLAG_N : 0);
}

bool Cpu::get_h_flag() {
    return this->f & FLAG_H;
}

void Cpu::set_h_flag(bool v) {
    this->f = (f & ~FLAG_H) | (v ? FLAG_H : 0);
}

bool Cpu::get_c_flag() {
    return this->f & FLAG_C;
}

void Cpu::set_c_flag(bool v) {
    this->f = (f & ~FLAG_C) | (v ? FLAG_C : 0);
}

Cpu::AluResult Cpu::alu_add(uint8_t lhs, uint8_t rhs, bool carry_in) {
    uint16_t sum = static_cast<uint16_t>(lhs) + rhs + (carry_in ? 1 : 0);
    AluResult r;
    r.value = static_cast<uint8_t>(sum);
    r.flags.z = ((sum & 0xff) == 0);
    r.flags.n = false;
    r.flags.h = (((lhs & 0xF) + (rhs & 0xF) + (carry_in ? 1 : 0)) > 0xF);
    r.flags.c = (sum > 0xff);
    return r;
}

void Cpu::apply_flags(Flags fl) {
    set_z_flag(fl.z);
    set_n_flag(fl.n);
    set_h_flag(fl.h);
    set_c_flag(fl.c);
}

void Cpu::load_flags(uint8_t v) {
    set_z_flag(v & FLAG_Z);
    set_n_flag(v & FLAG_N);
    set_h_flag(v & FLAG_H);
    set_c_flag(v & FLAG_C);
}

void Cpu::add8(uint8_t rhs) {
    auto r = alu_add(this->a, rhs, false);
    this->a = r.value;
    apply_flags(r.flags);
}

void Cpu::xor8(uint8_t rhs) {
    this->a ^= rhs;
    set_z_flag(this->a==0);
    set_n_flag(false);
    set_h_flag(false);
    set_c_flag(false);
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
        case 0x86:
            {
                uint16_t hl = static_cast<uint16_t>(h)<<8 | l;
                add8(bus.read(hl));
                cycles = 8;
            }
            break;
        case 0x87:
            add8(this->a);
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
