#include "bus.h"

Bus::Bus() {
    mem.fill(0xff);
}

uint8_t Bus::read(uint16_t addr) {
    return mem[addr];
}

void Bus::write(uint16_t addr, uint8_t value) {
    mem[addr] = value;
}
