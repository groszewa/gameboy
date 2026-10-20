#include "cpu.h"
#include <sstream>
#include "logging.h"

Cpu::Cpu(Bus& bus_) : bus(bus_){}

void Cpu::reset() {
    this->a = 0;
    this->f = 0;
    this->pc = 0;
}

void Cpu::step() {
    uint8_t opcode = bus.read(pc++);
    //GB_LOG("Current opcode is 0x%x\n", opcode);
    switch(opcode) {
        case 0xc6:
            {
                uint8_t imm = bus.read(this->pc++);
                uint16_t wide_result = static_cast<uint16_t>(this->a) + imm;

                // Set Flags
                // Zero (Z)
                if((wide_result&0xff)==0) this->f |= FLAG_Z;
                // Subtraction (N) - cannot be set for Add
                // Half Carry (H)
                if (((this->a & 0xf) + (imm & 0xf)) > 0xf) this->f |= FLAG_H;
                // Carry (C)
                if(wide_result > 0xff) this->f |= FLAG_C;

                this->a = static_cast<uint8_t>(wide_result);
                break;
            }
        case 0xee:
            {
                uint8_t imm = bus.read(this->pc++);
                uint16_t wide_result = static_cast<uint16_t>(this->a) ^ imm;

                // Set Flags
                // Zero (Z)
                if((wide_result&0xff)==0) this->f |= FLAG_Z;
                // Subtraction (N) - cannot be set for Xor
                // Half Carry (H) - cannot be set for Xor
                // Carry (C) - cannot be set for Xor

                this->a = static_cast<uint8_t>(wide_result);
                break;
            }
        default:
            std::ostringstream msg;
            msg << "Unknown opcode 0x" << std::hex << static_cast<int>(opcode);
            throw std::runtime_error(msg.str());
    }
}
