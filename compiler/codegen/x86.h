// This file is part of the Yu programming language and is licensed under MIT License;
// See LICENSE.txt for details

#pragma once

#include <vector>
#include <cstdint>
#include "../analyzer.h"

namespace yu::compiler
{
    class X86Generator
    {
    public:
        X86Generator(IRBuilder &builder, const IRAnalyzer &analyzer);

        struct x86_block_t
        {
            std::vector<uint8_t> machine_code;
            std::vector<uint32_t> instruction_sizes;
            std::vector<uint32_t> label_positions;
            std::vector<uint32_t> relocation_sites;
            std::vector<uint32_t> relocation_types;
        };

        struct PhiNode
        {
            uint32_t dest;
            uint32_t value;
            uint32_t pred_block;

            PhiNode(uint32_t d, uint32_t v, uint32_t p) : dest(d), value(v), pred_block(p) {}
        };

        x86_block_t generate_function(uint32_t function_index);

        std::vector<uint8_t> finalize();

        void emit_trunc(x86_block_t &block, uint32_t dest, uint32_t src);

    private:
        void emit_unreachable(x86_block_t &block);

        void emit_add(x86_block_t &block, uint32_t dest, uint32_t src1, uint32_t src2);

        void emit_sub(x86_block_t &block, uint32_t dest, uint32_t src1, uint32_t src2);

        void emit_mul(x86_block_t &block, uint32_t dest, uint32_t src1, uint32_t src2);

        void emit_div(x86_block_t &block, uint32_t dest, uint32_t src1, uint32_t src2);

        void emit_mov(x86_block_t &block, uint32_t dest, uint32_t src);

        static void emit_call(x86_block_t &block, uint32_t function_ptr);

        void emit_call_indirect(x86_block_t &block, uint32_t function_ptr);

        void emit_tail_call(x86_block_t &block, uint32_t function_ptr);

        void emit_fadd(x86_block_t &block, uint32_t dest, uint32_t src1, uint32_t src2);

        void emit_fsub(x86_block_t &block, uint32_t dest, uint32_t src1, uint32_t src2);

        void emit_fmul(x86_block_t &block, uint32_t dest, uint32_t src1, uint32_t src2);

        void emit_fdiv(x86_block_t &block, uint32_t dest, uint32_t src1, uint32_t src2);

        void emit_and(x86_block_t &block, uint32_t dest, uint32_t src);

        void emit_or(x86_block_t &block, uint32_t dest, uint32_t src);

        void emit_xor(x86_block_t &block, uint32_t dest, uint32_t src);

        void emit_shl(x86_block_t &block, uint32_t dest, uint32_t src);

        void emit_shr(x86_block_t &block, uint32_t dest, uint32_t src);

        void emit_sar(x86_block_t &block, uint32_t dest, uint32_t src);

        void emit_neg(x86_block_t &block, uint32_t dest);

        void emit_not(x86_block_t &block, uint32_t dest);

        void emit_eq(x86_block_t &block, uint32_t dest, uint32_t src1, uint32_t src2);

        void emit_ne(x86_block_t &block, uint32_t dest, uint32_t src1, uint32_t src2);

        void emit_lt(x86_block_t &block, uint32_t dest, uint32_t src1, uint32_t src2);

        void emit_le(x86_block_t &block, uint32_t dest, uint32_t src1, uint32_t src2);

        void emit_gt(x86_block_t &block, uint32_t dest, uint32_t src1, uint32_t src2);

        void emit_ge(x86_block_t &block, uint32_t dest, uint32_t src1, uint32_t src2);

        void emit_load(x86_block_t &block, uint32_t dest, uint32_t src);

        void emit_store(x86_block_t &block, uint32_t src, uint32_t dest);

        void emit_zext(x86_block_t &block, uint32_t dest, uint32_t src);

        void emit_sext(x86_block_t &block, uint32_t dest, uint32_t src);

        static void emit_jump(x86_block_t &block, uint32_t target);

        void emit_branch(x86_block_t &block, uint32_t condition, uint32_t target);

        void emit_return(x86_block_t &block, uint32_t value);

        uint32_t get_register_for_value(uint32_t value);

        static void emit_rex_prefix(x86_block_t &block, bool w_bit = false, uint32_t reg1 = 0, uint32_t reg2 = 0);

        static void emit_modrm(x86_block_t &block, uint8_t mod, uint8_t reg, uint8_t rm);

        void emit_memory_op(x86_block_t &block, uint32_t addr, int32_t offset, uint32_t index, uint8_t scale);

        void emit_atomic_load(x86_block_t &block, uint32_t dest, uint32_t src);

        void emit_atomic_store(x86_block_t &block, uint32_t dest, uint32_t src);

        void emit_atomic_add(x86_block_t &block, uint32_t dest, uint32_t src);

        void emit_atomic_sub(x86_block_t &block, uint32_t dest, uint32_t src);

        void emit_cmpxchg(x86_block_t &block, uint32_t dest, uint32_t expected, uint32_t new_val);

        void emit_mfence(x86_block_t &block);

        void emit_lfence(x86_block_t &block);

        void emit_sfence(x86_block_t &block);

        void emit_switch(x86_block_t &block, uint32_t value, const std::vector<uint32_t> &targets);

        void emit_phi(x86_block_t &block, uint32_t dest, const std::vector<std::pair<uint32_t, uint32_t> > &inputs);

        IRBuilder &builder;
        const IRAnalyzer &analyzer;
        std::vector<x86_block_t> code_blocks;

        std::vector<bool> register_in_use;
        std::vector<uint32_t> register_mapping;
        std::vector<PhiNode> phi_nodes;
    };
}
