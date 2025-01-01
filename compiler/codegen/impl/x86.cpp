#include "../x86.h"
#include <cassert>
#include <stdexcept>
#include <iostream>

namespace yu::compiler {

X86Generator::X86Generator(IRBuilder &builder, const IRAnalyzer &analyzer)
    : builder(builder)
    , analyzer(analyzer)
    , register_mapping(builder.get_instructions().destinations.size(), UINT32_MAX) {

    register_in_use = std::vector<bool>(16, false);

    // Mark system registers as used
    register_in_use[4] = true; // rsp
    register_in_use[5] = true; // rbp
    register_in_use[0] = true; // rax (return value)
}

uint32_t X86Generator::get_register_for_value(const uint32_t value)
{
    if (register_mapping[value] != UINT32_MAX) {
        return register_mapping[value];
    }

    // Find first free register
    for (uint32_t i = 0; i < register_in_use.size(); ++i) {
        if (!register_in_use[i]) {
            register_in_use[i] = true;
            register_mapping[value] = i;
            return i;
        }
    }

    throw std::runtime_error("No registers available");
}

void X86Generator::emit_rex_prefix(x86_block_t &block, bool w_bit, uint32_t reg1, uint32_t reg2) {
    uint8_t rex = 0x40;

    if (w_bit) {
        rex |= 0x08;
    }
    if (reg1 & 0x8) {
        rex |= 0x04; // REX.R extends ModR/M reg field
    }
    if (reg2 & 0x8) {
        rex |= 0x01; // REX.B extends ModR/M r/m field
    }

    block.machine_code.push_back(rex);
}

void X86Generator::emit_modrm(x86_block_t &block, uint8_t mod, uint8_t reg, uint8_t rm) {
    uint8_t modrm = (mod << 6) | ((reg & 0x7) << 3) | (rm & 0x7);
    block.machine_code.push_back(modrm);
}

void X86Generator::emit_add(x86_block_t &block, uint32_t dest, uint32_t src1, uint32_t src2) {
    std::cout << "Emitting ADD instruction" << std::endl;

    // Get a register for the result
    uint32_t dest_reg = get_register_for_value(dest);

    // First parameter is in rdi (7)
    emit_rex_prefix(block, true, dest_reg, 7);
    block.machine_code.push_back(0x89); // mov
    emit_modrm(block, 0b11, 7, dest_reg);

    // Add second parameter (rsi = 6)
    emit_rex_prefix(block, true, 6, dest_reg);
    block.machine_code.push_back(0x01); // add
    emit_modrm(block, 0b11, 6, dest_reg);
}

void X86Generator::emit_return(x86_block_t &block, uint32_t value) {
    std::cout << "Emitting RETURN instruction" << std::endl;

    // Move return value to rax if not already there
    uint32_t value_reg = get_register_for_value(value);
    if (value_reg != 0) { // 0 = rax
        emit_rex_prefix(block, true, 0, value_reg);
        block.machine_code.push_back(0x89); // mov
        emit_modrm(block, 0b11, value_reg, 0);
    }
}

X86Generator::x86_block_t X86Generator::generate_function(uint32_t function_index) {
    x86_block_t block;

    std::cout << "Generating function " << function_index << std::endl;

    // Prologue
    block.machine_code.push_back(0x55);        // push rbp
    block.machine_code.push_back(0x48);        // REX.W prefix
    block.machine_code.push_back(0x89);        // mov
    block.machine_code.push_back(0xe5);        // rbp, rsp

    // Get instruction information
    const auto &instructions = builder.get_instructions();
    const auto &bbs = builder.get_bbs();
    const auto &functions = builder.get_functions();

    uint32_t start_bb = functions.bb_start_indices[function_index];
    uint32_t bb_count = functions.bb_counts[function_index];

    // Process each basic block
    for (uint32_t bb = 0; bb < bb_count; ++bb) {
        uint32_t bb_idx = start_bb + bb;
        uint32_t start_inst = bbs.start_indices[bb_idx];
        uint32_t inst_count = bbs.instruction_counts[bb_idx];

        std::cout << "Processing BB " << bb << " with " << inst_count << " instructions" << std::endl;

        // Process each instruction
        for (uint32_t i = 0; i < inst_count; ++i) {
            uint32_t inst_idx = start_inst + i;
            auto op = instructions.ops[inst_idx];

            std::cout << "Processing instruction " << i << " op=" << static_cast<int>(op) << std::endl;

            switch (op) {
                case lang::ir_op_t::OP_ADD:
                    emit_add(block, instructions.destinations[inst_idx],
                            instructions.operands[inst_idx * 2],
                            instructions.operands[inst_idx * 2 + 1]);
                    break;

                case lang::ir_op_t::FLOW_RETURN:
                    emit_return(block, instructions.operands[inst_idx * 2]);
                    break;

                default:
                    throw std::runtime_error("Unsupported instruction");
            }
        }
    }

    // Epilogue
    block.machine_code.push_back(0xc9);        // leave
    block.machine_code.push_back(0xc3);        // ret

    code_blocks.push_back(block);
    return block;
}

std::vector<uint8_t> X86Generator::finalize() {
    std::vector<uint8_t> final_code;

    for (const auto &block : code_blocks) {
        final_code.insert(final_code.end(),
                         block.machine_code.begin(),
                         block.machine_code.end());
    }

    std::cout << "Final code size: " << final_code.size() << " bytes" << std::endl;
    return final_code;
}

}