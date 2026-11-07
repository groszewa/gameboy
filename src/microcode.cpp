#include <format>
#include <fstream>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include "microcode.h"
#include "logging.h"

Microcode::Microcode(const std::string &path) {
    std::ifstream json_file(path);
    if(!json_file) {
        throw std::runtime_error("Cannot open opcodes.json");
    }
    auto json_table = nlohmann::json::parse(json_file);
    auto fill = [](const nlohmann::json& table, std::array<OpcodeInfo, 256>& out) {
        for(int opcode = 0; opcode < 256; opcode++) {
            const auto& entry = table[std::format("0x{:02X}", opcode)];
            const auto& cycles = entry["cycles"];
            std::string mnemonic = entry["mnemonic"];
            out[opcode].cycles     = cycles[0];
            out[opcode].cycles_alt = cycles.size() > 1 ? cycles[1] : cycles[0];
            out[opcode].bytes      = entry["bytes"];
            out[opcode].mnemonic   = mnemonic;
            out[opcode].illegal    = mnemonic.starts_with("ILLEGAL");
        }
    };
    fill(json_table["unprefixed"], unprefixed);
    fill(json_table["cbprefixed"], cbprefixed);
}
